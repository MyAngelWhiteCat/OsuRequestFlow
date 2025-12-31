#pragma once

#include "core/core.h"
#include "api_request_handler.h"
#include "file_request_handler.h"


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

namespace gui_http {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
    public:
        RequestHandler(core::Core& core, const std::filesystem::path& root, Strand& api_strand)
            : core_(core)
            , file_handler_(root)
            , api_strand_(api_strand)
        {

        }

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            if (CORS(req, send)) {
                return;
            }
            std::string_view target = req.target();
            if (target.substr(0, api_prefix_.size()) == api_prefix_) {
                net::dispatch(api_strand_, 
                    [self = this->shared_from_this(), req = std::move(req), send = std::move(send)]() mutable {
                    self->api_handler_(std::move(req), std::move(send)); 
                    });
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
        Strand& api_strand_;

        template <typename Body, typename Allocator, typename Send>
        bool CORS(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
            if (req.method() == http::verb::options) {
                http::response<http::string_body> res{ http::status::ok, req.version() };
                res.set(http::field::access_control_allow_origin, "*");
                res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
                res.set(http::field::access_control_allow_headers, "Content-Type");
                res.set(http::field::content_length, "0");
                res.keep_alive(req.keep_alive());
                send(std::move(res));
                return true;
            }
            return false;
        }
    };

}