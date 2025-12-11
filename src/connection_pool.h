#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/connect.hpp>

#include <boost/beast.hpp>

#include <memory>
#include <variant>
#include <string_view>
#include <stdexcept>
#include <string>

#include "logging.h"
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/error.hpp>

namespace http_domain {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace ssl = net::ssl;

    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using StringResponse = http::response<http::string_body>;
    using StringRequest = http::request<http::string_body>;
    using DynamicResponse = http::response<http::dynamic_body>;


    class Connection {
    public:

        Connection(net::io_context& ioc)
            : stream_(beast::tcp_stream(ioc))
        {

        }

        Connection(net::io_context& ioc, std::shared_ptr<ssl::context> ctx)
            : ctx_(ctx)
            , stream_(ssl::stream<beast::tcp_stream>(ioc, *ctx))
        {

        }

        auto& GetStream() {
            return stream_;
        }

        void ConnectionReset() {
            ssl_connected_ = false;
            connected_ = false;
        }

        bool Connect(std::string_view host, std::string_view port) {
            ec_.clear();

            ConnectionVisitor visitor(*this, host, port);
            std::visit(visitor, stream_);
            if (ec_) {
                logging::ReportError(ec_, "Connection");
                return false;
            }
            return true;
        }

        bool IsConnected() const {
            return ssl_connected_ || connected_;
        }

    private:
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        std::variant<beast::tcp_stream, ssl::stream<beast::tcp_stream>> stream_;
        beast::error_code ec_;

        bool ssl_connected_ = false;
        bool connected_ = false;


        class GetterVisitor {
        public:
            beast::tcp_stream& operator()(beast::tcp_stream& stream) {
                return stream;
            }

            ssl::stream<beast::tcp_stream>& operator()(ssl::stream<beast::tcp_stream>& stream) {
                return stream;
            }

        };

        class ConnectionVisitor {
        public:
            explicit ConnectionVisitor(Connection& connection, std::string_view host, std::string_view port)
                : connection(connection)
                , host_(host)
                , port_(port)
            {
            }

            void operator()(beast::tcp_stream& socket) {
                tcp::resolver resolver(socket.get_executor()); 
                auto endpoints = resolver.resolve(host_, port_);
                if (connection.ec_) {
                    logging::ReportError(connection.ec_, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                    for (const auto& ep : endpoints) {
                        LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                    }
                }

                socket.connect(endpoints, connection.ec_);
                if (connection.ec_) {
                    logging::ReportError(connection.ec_, "Connection"sv);
                    return;
                }
                connection.connected_ = true;
                LOG_INFO("CONNECTED");
            }

            void operator()(ssl::stream<beast::tcp_stream>& socket) {
                tcp::resolver resolver(socket.get_executor());
                auto endpoints = resolver.resolve(host_, port_, connection.ec_);

                if (connection.ec_) {
                    logging::ReportError(connection.ec_, "Resolving");
                    throw std::runtime_error("cant resolve: "s.append(host_).append(" ").append(port_));
                }

                LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                for (const auto& ep : endpoints) {
                    LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                }

                // AI on
                SSL_set_tlsext_host_name(socket.native_handle(), host_.c_str());
                // AI off

                net::connect(socket.lowest_layer(), endpoints, connection.ec_);
                if (connection.ec_) {
                    logging::ReportError(connection.ec_, "SSL Connection"sv);
                    return;
                }
                else {
                    LOG_INFO("CONNECTED");
                }

                // AI on
                socket.lowest_layer().set_option(tcp::no_delay(true));
                // AI off

                socket.handshake(ssl::stream_base::client, connection.ec_);
                if (connection.ec_) {
                    ERR_print_errors_fp(stderr);

                    logging::ReportError(connection.ec_, "SSL Handshake");
                    socket.lowest_layer().close();
                    return;
                }

                else {
                    LOG_INFO("HANDSHAKE SUCESS");
                }
                connection.ssl_connected_ = true;

                // AI on
                if (SSL_get_verify_result(socket.native_handle()) != X509_V_OK) {
                    LOG_ERROR("SSL Certificate verification failed");
                }
                else {
                    LOG_INFO("SSL Certificate verified successfully");
                }
                // AI off

            }

        private:
            Connection& connection;
            std::string host_;
            std::string port_;
        };

    };


}