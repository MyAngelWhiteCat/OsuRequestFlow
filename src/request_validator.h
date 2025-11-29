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
        std::optional<std::string> ValidateDownloadsFolderRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<std::string> ValidateListEditRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<std::string> ValidateJoinRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<std::string> ValidatePartRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<std::pair<std::string, std::string>> ValidateSetResourseRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<int> ValidateSetRoleFilterRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<int> ValidateReconnectTimeoutRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        std::optional<bool> ValidateSetWhiteListOnlyRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send);

        template<typename Body, typename Allocator, typename Send>
        void SendMethodNotAllowed(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view allowed);

        template<typename Body, typename Allocator, typename Send>
        void SendInvalidArgument(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info);

    private:
        response_maker::ResponseMaker& response_maker_;

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
    inline std::optional<std::string> RequestValidator::ValidateDownloadsFolderRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
            return std::nullopt;
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("Path") && (*parsed_body)["Path"].is_string()) {
                std::string path = (*parsed_body)["Path"].get<std::string>();
                if (std::filesystem::exists(path)) {
                    return path;
                }
                SendInvalidArgument(req, send, "Path does not exist");
                return std::nullopt;
            }
            SendInvalidArgument(req, send, "Invalid path");
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<std::string> RequestValidator::ValidateListEditRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("UserName") && (*parsed_body)["UserName"].is_string()) {
                return (*parsed_body)["UserName"].get<std::string>();
            }
            SendInvalidArgument(req, send, "Invalid UserName");
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<std::string> RequestValidator::ValidateJoinRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::post) {
            SendMethodNotAllowed(req, send, "POST");
            return std::nullopt;
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("Channel") && (*parsed_body)["Channel"].is_string()) {
                return (*parsed_body)["Channel"].get<std::string>();
            }
            SendInvalidArgument(req, send, "Invalid channel");
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<std::string> RequestValidator::ValidatePartRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        return ValidateJoinRequest(req, send);
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<std::pair<std::string, std::string>> RequestValidator::ValidateSetResourseRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
            return std::nullopt;
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("Resourse") && parsed_body->contains("Prefix")
                && (*parsed_body)["Resourse"].is_string() && (*parsed_body)["Prefix"].is_string()) {
                return std::make_pair((*parsed_body)["Resourse"].get<std::string>(), (*parsed_body)["Prefix"].get<std::string>());
            }
            SendInvalidArgument(req, send, "Invalid resourse or prefix");
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<int> RequestValidator::ValidateSetRoleFilterRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
            return std::nullopt;
        }
        json j;
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("RoleFilterLevel") && (*parsed_body)["RoleFilterLevel"].is_number()) {
                int lvl = (*parsed_body)["RoleFilterLevel"].get<int>();
                if (lvl >= 0 && lvl <= 4) {
                    return lvl;
                }
                SendInvalidArgument(req, send, "Invalid level. Expected 0 - 4");
                return std::nullopt;
            }
            SendInvalidArgument(req, send, "Invalid level");
        }
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<int> RequestValidator::ValidateReconnectTimeoutRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("ReconnectTimeout") && (*parsed_body)["ReconnectTimeout"].is_number_unsigned()) {
                return (*parsed_body)["ReconnectTimeout"].get<unsigned>();
            }
        }
        SendInvalidArgument(req, send, "Invalid time");
        return std::nullopt;
    }

    template<typename Body, typename Allocator, typename Send>
    inline std::optional<bool> RequestValidator::ValidateSetWhiteListOnlyRequest(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
        if (req.method() != http::verb::put) {
            SendMethodNotAllowed(req, send, "PUT");
        }
        if (auto parsed_body = ParseJson(req, send)) {
            if (parsed_body->contains("IsOn") && (*parsed_body)["IsOn"].is_boolean()) {
                return (*parsed_body)["IsOn"].get<bool>();
            }
        }
        SendInvalidArgument(req, send, "Invalid status");
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
            std::cout << obj.dump(4) << std::endl;
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