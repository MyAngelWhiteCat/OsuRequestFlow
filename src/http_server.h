#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <memory>
#include <iostream>

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
            request_handler_(std::move(request), [self = this->shared_from_this()](auto&& response) {
                self->Write(std::move(response));
                });
        }

    };

    template <typename RequestHandler>
    class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
    public:
        template <typename Handler>
        Listener(net::io_context& ioc, const tcp::endpoint& endpoint, Handler&& handler)
            : ioc_(ioc)
            , acceptor_(net::make_strand(ioc))
            , request_handler_(std::forward<Handler>(handler))
        {
            acceptor_.open(endpoint.protocol());
            acceptor_.set_option(net::socket_base::reuse_address(true));
            acceptor_.bind(endpoint);
            acceptor_.listen(net::socket_base::max_listen_connections);
        }

        void Run() {
            DoAccept();
        }

    private:
        net::io_context& ioc_;
        tcp::acceptor acceptor_;
        RequestHandler request_handler_;

        void DoAccept() {
            acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&Listener::OnAccept, this->shared_from_this()));
        }

        void OnAccept(boost::system::error_code ec, tcp::socket socket) {
            using namespace std::literals;

            if (ec) {
                return logging::ReportError(ec, "accept"sv);
            }

            AsyncRunSession(std::move(socket));

            DoAccept();
        }

        void AsyncRunSession(tcp::socket&& socket) {
            std::make_shared<Session<RequestHandler>>(std::move(socket), request_handler_)->Run();
        }
    };

    template <typename RequestHandler>
    void ServeHttp(net::io_context& ioc, const tcp::endpoint& endpoint, RequestHandler&& handler) {
        using MyListener = Listener<std::decay_t<RequestHandler>>;
        std::make_shared<MyListener>(ioc, endpoint, std::move(handler))->Run();
    }



}