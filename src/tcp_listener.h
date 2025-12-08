#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <memory>
#include <utility>
#include <string_view>

#include "logging.h"
#include <boost/system/detail/error_code.hpp>

namespace tcp_listener {

    namespace net = boost::asio;
    using tcp = net::ip::tcp;
    namespace beast = boost::beast;


    template <typename RequestHandler, typename Session>
    class Listener : public std::enable_shared_from_this<Listener<RequestHandler, Session>> {
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
            std::make_shared<Session>(std::move(socket), request_handler_)->Run();
        }
    };

}