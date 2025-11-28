#pragma once

#include "core.h"
#include "api_request_handler.h"
#include "file_request_handler.h"


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

namespace gui_http {

    class RequestHandler {
    public:
        RequestHandler(core::Core& core, const std::filesystem::path& root)
            : core_(core)
            , file_handler_(root)
        {

        }

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            std::string_view target = req.target();
            if (target.substr(0, api_prefix_.size()) == api_prefix_) {
                api_handler_(std::move(req), std::move(send)); // NEED TO DISPATCH IN API STRAND!!!
            }
            else {
                file_handler_(std::move(req), std::move(send));
            }
        }

    private:
        core::Core& core_;
        FileRequestHandler file_handler_;
        ApiRequestHandler api_handler_{ core_ };
        const std::string api_prefix_ = "/api/";

    };

}