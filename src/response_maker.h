#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <iostream>

#include "domain_fields.h"

namespace response_maker {

    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;

    using json = nlohmann::json;
    using namespace std::literals;

    class ResponseMaker {
    public:

        json GetBadRequest();
        json MakeBadRequest(std::string_view key, std::string_view value);

        std::string_view GetContentType(const fs::path& uri);

        template <typename BodyType, typename BodyValue>
        auto MakeBaseResponse(
            http::status status,
            unsigned http_version,
            BodyValue&& body_value,
            bool keep_alive,
            std::string_view content_type = fields::ContentType::APPLICATION_JSON,
            std::string_view allow_methods = ""sv)
        {
            http::response<BodyType> response(status, http_version);
            response.set(http::field::content_type, content_type);
            if (!allow_methods.empty()) {
                response.set(http::field::allow, allow_methods);
            }
            response.keep_alive(keep_alive);
            response.set(http::field::cache_control, "no-cache"sv);

            try {
                response.body() = std::forward<BodyValue>(body_value);
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }

            response.content_length(response.body().size());
            return response;
        }

        
        auto MakeStringResponse(
            http::status status,
            unsigned http_version,
            json&& body,
            bool keep_alive,
            std::string_view allowed = "GET, HEAD",
            std::string_view content_type = fields::ContentType::APPLICATION_JSON)
        {
            return MakeBaseResponse<http::string_body>(
                status, http_version, std::move(body.dump(4)), keep_alive, content_type, allowed);
        }

        auto MakeFileResponse(
            http::status status,
            unsigned http_version,
            http::file_body::value_type file,
            bool keep_alive,
            const std::filesystem::path& content_uri)
        {
            auto response = MakeBaseResponse<http::file_body>(
                status, http_version, std::move(file), keep_alive, GetContentType(content_uri), "GET, HEAD"sv);
            return response;
        }

    };

}