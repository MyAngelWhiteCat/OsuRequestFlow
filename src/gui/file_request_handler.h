#pragma once

#include "http_server/root_directory.h"
#include "gui/response_maker.h"
#include <filesystem>


namespace gui_http {


    using namespace std::literals;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace fs = std::filesystem;


    class FileRequestHandler {
    public:
        FileRequestHandler(const std::filesystem::path root)
            : root_(root)
        {
        }

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            std::string req_target = req.target();
            auto file = root_.GetContent(fs::path(req_target));
            http::response<http::string_body> responce;
            http::response<http::file_body> file_responce;
            switch (file.status)
            {
            case root_directory::ContentStatus::OK:
                file_responce = response_maker_.MakeFileResponse(http::status::ok, req.version(), std::move(file.file), req.keep_alive(), fs::path(req_target));
                return send(std::move(file_responce));
                break;
            case root_directory::ContentStatus::BAD_ACCESS:
                responce = response_maker_.MakeStringResponse(http::status::bad_request, req.version(), response_maker_.GetBadRequest(), req.keep_alive(), fields::ContentType::TEXT_PLAIN);
                break;
            case root_directory::ContentStatus::NOT_FOUND:
                responce = response_maker_.MakeStringResponse(http::status::not_found, req.version(), "File not found", req.keep_alive(), fields::ContentType::TEXT_PLAIN);
                break;
            default:
                break;
            }
            send(std::move(responce));
        }

    private:
        root_directory::RootDirectory root_;
        response_maker::ResponseMaker response_maker_;
    };

}