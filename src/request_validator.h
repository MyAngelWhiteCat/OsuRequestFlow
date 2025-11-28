#pragma once

#include "response_maker.h"

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <optional>
#include <string_view>

namespace request_validator {

    
    namespace beast = boost::beast;
    namespace http = beast::http;
    
    using json = nlohmann::json;

    class RequestValidator {
    public:
        RequestValidator(response_maker::ResponseMaker& response_maker)
            : response_maker_(response_maker)
        {
        }

        template <typename Body, typename Allocator, typename Send>
        bool ValidateSettingsRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<size_t> ValidateFileSizeRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        std::optional<std::string_view> ValidateDownloadsFolderRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

    private:
        response_maker::ResponseMaker& response_maker_;

        template<typename Body, typename Allocator, typename Send>
        void SendMethodNotAllowed(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view allowed);

        template<typename Body, typename Allocator, typename Send>
        void SendInvalidArgument(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info);

        template<typename Body, typename Allocator, typename Send>
        std::optional<json> ParseJson(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);
    };

    template<typename Body, typename Allocator, typename Send>
    inline bool RequestValidator::ValidateSettingsRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::post) {
            SendMethodNotAllowed(req, send, "POST");
            return false;
        }
        return true;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<size_t> RequestValidator::ValidateFileSizeRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
            return std::nullopt;
        }
        
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("FileSize") && (*parsed_body)["FileSize"].is_number_unsigned()) {
                return (*parsed_body)["FileSize"].get<size_t>();
            }
            else {
                SendInvalidArgument(req, send, "Invalid argument. Expected {\"FileSize\":unsigned}");
            }
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<std::string_view> RequestValidator::ValidateDownloadsFolderRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
            return std::nullopt;
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("Path") && (*parsed_body)["Path"].is_string()) {
                std::string_view path = (*parsed_body)["Path"].get<std::string_view>();
                if (std::filesystem::exists(path)) {
                    return path;
                }
            }
        }
        SendInvalidArgument(req, send, "Invalid path");
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline void RequestValidator::SendMethodNotAllowed(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view allowed) {
        send(response_maker_.MakeStringResponse(
            http::status::method_not_allowed,
            req.version(),
            response_maker_.MakeBadRequest(fields::RequestError::INVALID_METHOD, "Method not allowed"),
            req.keep_alive(),
            allowed
        ));
    }

    template<typename Body, typename Allocator, typename Send>
    inline void RequestValidator::SendInvalidArgument(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info) {
        send(response_maker_.MakeStringResponse(
            http::status::bad_request,
            req.version(),
            response_maker_.MakeBadRequest(fields::RequestError::INVALID_ARGUMENT, info),
            req.keep_alive()
        ));
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<json> RequestValidator::ParseJson(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        json obj;
        try {
            obj = json::parse(req.body());
        }
        catch (const std::exception& e) {
            send(response_maker_.MakeStringResponse(
                http::status::bad_request,
                req.version(),
                response_maker_.MakeBadRequest(fields::RequestError::INVALID_ARGUMENT, "Wrong JSON format"),
                req.keep_alive()
            ));
            return std::nullopt;
        }
        return obj;
    }


}