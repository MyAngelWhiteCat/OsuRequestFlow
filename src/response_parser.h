#pragma once

#include <string>
#include <string_view>
#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/beast.hpp>

#include <string_view>
#include <string>
#include <utility>
#include <fstream>
#include "logging.h"

#include "decode_url.h"


namespace http_domain {

    constexpr size_t KiB = 1024;
    constexpr size_t MiB = 1024 * KiB;
    constexpr size_t GiB = 1024 * MiB;

    struct Fields {
        static constexpr std::string_view CONTENT_LENGTH = "Content-Length"sv;
        static constexpr std::string_view CONTENT_DISPOSITION = "Content-Disposition"sv;
        static constexpr std::string_view LOCATION = "Location"sv;
    };

    struct Status {
        static constexpr std::string_view OK = "OK"sv;
    };

    class ResponseParser {
    public:

        ResponseParser(int max_file_size_MiB)
            : max_file_size_(MiB * max_file_size_MiB)
        {
            response_parser_.body_limit(max_file_size_);
        }

        bool IsOK() {
            auto& response = response_parser_.get();
            return response.reason() == Status::OK;
        }

        void PrintResponseHeaders() {
            auto& response = response_parser_.get();
            LOG_INFO(response.reason());
            std::ofstream log_request("LogRequest.txt", std::ios::app);
            log_request << "RESPONSE" << "\n";
            for (const auto& header : response) {
                log_request << header.name_string() << ": " << header.value() << "\n";
            }
            log_request << "\n\n";
        }

        std::string GetFileName() {
            auto& response = response_parser_.get();
            auto it = response.find(Fields::CONTENT_DISPOSITION);
            if (it != response.end()) {
                return ParseFileName(it->value());
            }
            return "JohnDoe";
        }

        void CheckRedirect() {
            auto& headers = response_parser_.get();
            auto it = headers.find(Fields::LOCATION);
            if (it != headers.end()) {
                // TODO: ProcessRedirect(it->value());
                LOG_INFO("Redirect to: " + std::string(it->value()));
            }
        }

        bool CheckFileSize() {
            auto& response = response_parser_.get();
            auto it = response.find(Fields::CONTENT_LENGTH);
            if (it != response.end()) {
                file_size_ = std::stoll(it->value());
                if (file_size_ < max_file_size_) {
                    response_parser_.body_limit(file_size_ + 10 * KiB);
                    return true;
                }
                LOG_ERROR("Filesize " + std::to_string(file_size_ / MiB) + "MiB > Max file size " + std::to_string(max_file_size_ * MiB));
            }
            else {
                LOG_INFO("UNDEFINED CONTENT LENGTH");
            }
            return false;
        }

        http::response_parser<http::dynamic_body>& GetParser() {
            return response_parser_;
        }

        auto& GetBody() {
            return response_parser_.get().body();
        }

        std::vector<char> GetBodyBytes() {
            return std::move(body_bytes_);
        }

        void ParseResponseBody() {
            dynamic_response_ = response_parser_.release();
            body_bytes_.clear();
            auto buffers = dynamic_response_.body().data();
            for (auto it = buffers.begin(); it != buffers.end(); ++it) {
                auto buffer = *it;
                const char* data = static_cast<const char*>(buffer.data());
                body_bytes_.insert(body_bytes_.end(), data, data + buffer.size());
            }
        }

        void SetMaxFileSize(size_t new_max_file_size) {
            max_file_size_ = new_max_file_size;
            response_parser_.body_limit(max_file_size_);
        }

        size_t GetFileSize() {
            return file_size_;
        }

        double GetProgress() {
            return static_cast<double>(response_parser_.get().body().size()) / static_cast<double>(file_size_) * 100;
        }

    private:
        http::response_parser<http::dynamic_body> response_parser_;
        DynamicResponse dynamic_response_;
        std::vector<char> body_bytes_;
        size_t file_size_ = 0;
        size_t max_file_size_ = 200 * MiB;


        std::string ParseFileName(std::string_view content) {
            size_t start_pos = content.find_first_of('"') + 1;
            size_t end_pos = content.find_last_of('"');
            if (start_pos != std::string::npos && end_pos != std::string::npos) {
                return DecodeURL(content.substr(start_pos, end_pos - start_pos));
            }
            return "JohnDoe";
        }

    };

}