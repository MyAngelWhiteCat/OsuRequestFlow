#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <memory>
#include <iostream>

#include "tcp_listener.h"
#include "logging.h"


namespace http_server {

    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    namespace beast = boost::beast;
    namespace http = beast::http;

    class SessionBase {
    public:

        SessionBase(const SessionBase& other) = delete;
        SessionBase operator=(const SessionBase& other) = delete;

        void Run();

    protected:
        using HttpRequest = http::request<http::string_body>;

        explicit SessionBase(tcp::socket&& socket)
            : stream_(std::move(socket))
        {
        }

        template <typename Body, typename Fields>
        void Write(http::response<Body, Fields>&& responce) {
            auto safe_responce = std::make_shared<http::response<Body, Fields>>(std::move(responce));
            auto self = GetSharedThis();
            http::async_write(stream_, *safe_responce, [safe_responce, self](beast::error_code ec, std::size_t bytes_writen) {
                self->OnWrite(safe_responce->need_eof(), ec, bytes_writen);
                });
        }

        ~SessionBase() = default;

    private:
        beast::tcp_stream stream_;
        beast::flat_buffer buffer_;
        HttpRequest request_;

        void OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_writen);

        void Read();

        virtual std::shared_ptr<SessionBase> GetSharedThis() = 0;

        void OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read);

        void Close();
        virtual void HandleRequest(HttpRequest&& request) = 0;
    };

    template <typename RequestHandler>
    class Session : public SessionBase, public std::enable_shared_from_this<Session<RequestHandler>> {
    public:
        template <typename Handler>
        Session(tcp::socket&& socket, Handler&& handler)
            : SessionBase(std::move(socket))
            , request_handler_(std::forward<Handler>(handler))
        {
        }

    private:
        RequestHandler request_handler_;

        std::shared_ptr<SessionBase> GetSharedThis() override {
            return this->shared_from_this();
        }

        void HandleRequest(HttpRequest&& request) override {
            try {
                request_handler_(std::move(request), [self = this->shared_from_this()](auto&& response) {
                    self->Write(std::move(response));
                    });
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
            }
        }

    };


    template <typename RequestHandler>
    void ServeHttp(net::io_context& ioc, const tcp::endpoint& endpoint, RequestHandler&& handler) {
        using MyListener = tcp_listener::Listener<RequestHandler, Session<RequestHandler>>;
        std::make_shared<MyListener>(ioc, endpoint, std::move(handler))->Run();
    }



}