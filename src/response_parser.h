#pragma once

#include <string>
#include <string_view>
#include <iostream>
#include <filesystem>

#include <boost/beast.hpp>

#include <string_view>
#include <string>
#include "logging.h"

#include "decode_url.h"
#include <optional>


namespace http_domain {

    using namespace std::literals;

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

    using FileResponse = http::response<http::file_body>;

    template <typename Parser>
    class FileResponseParser {
    public:

        FileResponseParser(int max_file_size_MiB)
            : max_file_size_(MiB* max_file_size_MiB)
        {
            LOG_DEBUG("File response parser constructed");
            response_parser_.body_limit(max_file_size_);
        }

        bool IsOK() {
            auto& response = response_parser_.get();
            return response.reason() == Status::OK;
        }

        void SetRootDirectory(const std::filesystem::path& root_dir) {
            root_directory_ = root_dir;
        }

        void PrintResponseHeaders() {
            auto& response = response_parser_.get();

            LOG_INFO(response.reason());
            std::ofstream log_request("LogRequest.txt", std::ios::app);
            log_request << "RESPONSE" << "\n";
            log_request << "Status: " << response.reason() << "\n";
            for (const auto& header : response) {
                log_request << header.name_string() << ": " << header.value() << "\n";
            }
            log_request << "\n\n";
            LOG_INFO("Save respose in log file");
        }

        std::string_view GetFileName() {
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

        Parser& GetParser() {
            return response_parser_;
        }

        void SetMaxFileSize(size_t new_max_file_size) {
            max_file_size_ = new_max_file_size;
            response_parser_.body_limit(max_file_size_);
        }

        size_t GetFileSize() const {
            return file_size_;
        }

        double GetProgress() {
            if constexpr (std::is_same_v<Parser, http::response_parser<http::file_body>>) {
                return static_cast<double>(std::filesystem::file_size(GetFilePath()))
                    / static_cast<double>(file_size_) * 100;
            }
            return static_cast<double>(response_parser_.get().body().size())
                / static_cast<double>(file_size_) * 100;
        }

        bool OpenFile() {
            if constexpr (std::is_same_v<Parser, http::response_parser<http::file_body>>) {
                beast::error_code ec;
                GetFileName();
                response_parser_.get().body().open(GetFilePath().string().c_str(),
                    beast::file_mode::write, ec);
                return !ec;
            }
            else {
                return true;
            }
        }

        void SetSpeedMesureMode(bool is_speed_mesuring) {
            LOG_DEBUG("Speed mesure mode seted as "s.append(std::to_string(is_speed_mesuring)));
            speed_mesure_mode_ = is_speed_mesuring;
        }

    private:
        std::optional<std::filesystem::path> root_directory_;
        Parser response_parser_;
        bool speed_mesure_mode_ = false;
        size_t file_size_ = 0;
        size_t max_file_size_ = 200 * MiB;
        std::string file_name_ = "JohnDoe";


        std::string_view ParseFileName(std::string_view content) {
            size_t start_pos = content.find_first_of('"') + 1;
            size_t end_pos = content.find_last_of('"');
            if (start_pos != std::string::npos && end_pos != std::string::npos) {
                file_name_ = DecodeURL(content.substr(start_pos, end_pos - start_pos));
            }
            LOG_DEBUG("File name: "s.append(file_name_));
            return file_name_;
        }

        std::filesystem::path GetFilePath() const {
            if (!root_directory_) {
                throw std::runtime_error("Root directory not seted");
            }
            return std::filesystem::path(*root_directory_ / file_name_);
        }

    };


}