#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <memory>
#include <stdexcept>
#include <optional>

#include "domain.h"
#include "message.h"
#include "logging.h"
#include "auth_data.h"
#include "message_processor.h"
#include "ca_sertificates_loader.h"


namespace connection {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    static std::shared_ptr<ssl::context> GetSSLContext() {
        auto ctx = std::make_shared<ssl::context>(ssl::context::tlsv12_client);
        ctx->set_verify_mode(ssl::verify_peer);

        // AI on
        try {
            ctx->set_default_verify_paths();
        }
        catch (...) {}
        ssl_domain_utilities::load_windows_ca_certificates(*ctx);
        // AI off

        return ctx;
    }

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection(net::io_context& ioc, Strand& write_strand, Strand& read_strand)
            : write_strand_(read_strand)
            , read_strand_(read_strand)
            , socket_(tcp::socket(ioc))
            , ioc_(&ioc)
            , secured_(false)
        {

        }

        Connection(net::io_context& ioc, ssl::context& ctx, Strand& write_strand, Strand& read_strand)
            : write_strand_(read_strand)
            , read_strand_(read_strand)
            , socket_(ssl::stream<tcp::socket>(ioc, ctx))
            , ioc_(&ioc)
            , secured_(true)
        {
            
        }

        void Connect(std::string_view host, std::string_view port) {
            ec_.clear();

            ConnectionVisitor visitor(*this, host, port);
            std::visit(visitor, socket_);
            if (ec_) {
                logging::ReportError(ec_, "Connection");
            }
        }

        void Disconnect(bool is_need_to_close_socket = true) {
            ec_.clear();

            DisconnectVisitor visitor(*this, is_need_to_close_socket);
            std::visit(visitor, socket_);
            if (ec_) {
                logging::ReportError(ec_, "Disconnecting");
            }
            LOG_INFO("Disconnected");
        }

        bool IsReconnectRequired() {
            if (reconnect_required_) {
                reconnect_required_ = false;
                return true;
            }
            return false;
        }

        template <typename Handler>
        void AsyncRead(Handler&& handler) {
            auto visitor = std::make_shared<AsyncReadVisitor<Handler>>(
                this->shared_from_this(), std::forward<Handler>(handler));

            std::visit(*visitor, socket_);
        }

        void Write(std::string_view data) {
            ec_.clear();
            auto visitor = std::make_shared<WriteVisitor>(
                this->shared_from_this(), data);

            std::visit(*visitor, socket_);
            if (ec_) {
                logging::ReportError(ec_, "Writing");
            }
        }

        template <typename Handler>
        void AsyncWrite(std::string_view data, Handler&& callback) {
            auto visitor = std::make_shared<AsyncWriteVisitor<Handler>>(
                this->shared_from_this(), data, std::forward<Handler>(callback));

            std::visit(*visitor, socket_);
        }

        void AsyncWrite(std::string_view data) {
            AsyncWrite(data, [](const sys::error_code& ec) {
                if (ec) {
                    logging::ReportError(ec, "Writing");
                    }
                });
        }

        bool IsConnected() const {
            return ssl_connected_ || connected_;
        }

        net::io_context* GetContext() {
            return ioc_;
        }

        bool IsSecured() const {
            return secured_;
        }

    private:
        Strand& write_strand_;
        Strand& read_strand_;
        sys::error_code ec_;
        std::variant<tcp::socket, ssl::stream<tcp::socket>> socket_;

        bool ssl_connected_ = false;
        bool secured_ = false;
        bool connected_ = false;
        bool reconnect_required_ = false;

        net::io_context* ioc_ = nullptr;

        class ConnectionVisitor {
        public:
            explicit ConnectionVisitor(Connection& connection, std::string_view host, std::string_view port)
                : connection_(connection)
                , host_(host)
                , port_(port)
            {
            }

            void operator()(tcp::socket& socket) {
                tcp::resolver resolver(socket.get_executor()); // :(
                auto endpoints = resolver.resolve(host_, port_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                    for (const auto& ep : endpoints) {
                        LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                    }
                }
                net::connect(socket, endpoints, connection_.ec_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "Connection"sv);
                    return;
                }
                connection_.connected_ = true;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                tcp::resolver resolver(socket.get_executor()); // :(
                auto endpoints = resolver.resolve(host_, port_, connection_.ec_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                    for (const auto& ep : endpoints) {
                        LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                    }
                }
                net::connect(socket.lowest_layer(), endpoints, connection_.ec_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "SSL Connection"sv);
                    return;
                }
                else {
                    LOG_INFO("CONNECTED");
                }
                socket.lowest_layer().set_option(tcp::no_delay(true));
                socket.handshake(ssl::stream_base::client, connection_.ec_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "SSL Handshake"sv);
                    socket.lowest_layer().close();
                    return;
                }
                else {
                    LOG_INFO("HANDSHAKE SUCESS");
                }
                connection_.ssl_connected_ = true;
            }

        private:
            Connection& connection_;
            std::string host_;
            std::string port_;
        };

        class DisconnectVisitor {
        public:
            explicit DisconnectVisitor(Connection& connection, bool is_need_to_close_socket)
                : connection_(connection)
                , is_need_to_close_socket_(is_need_to_close_socket)
            {
            }

            void operator()(tcp::socket& socket) {
                socket.shutdown(net::socket_base::shutdown_send, ignor_);
                if (is_need_to_close_socket_) {
                    socket.close(connection_.ec_);
                }
                connection_.connected_ = false;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                socket.shutdown(ignor_);
                if (is_need_to_close_socket_) {
                    socket.lowest_layer().close(connection_.ec_);
                }
                connection_.ssl_connected_ = false;
            }

        private:
            Connection& connection_;
            sys::error_code ignor_;
            bool is_need_to_close_socket_;
        };

        template <typename Handler>
        class AsyncReadVisitor : public std::enable_shared_from_this<AsyncReadVisitor<Handler>> {
        public:

            using HandlerType = typename std::decay<Handler>::type;

            AsyncReadVisitor(const AsyncReadVisitor&) = delete;
            AsyncReadVisitor& operator=(const AsyncReadVisitor&) = delete;

            explicit AsyncReadVisitor(std::weak_ptr<Connection> connection
                , Handler&& handler
                , size_t read_buffer_size = 512)
                : connection_(connection)
                , handler_(std::forward<Handler>(handler))
                , read_buffer_size_(read_buffer_size)
            {
            }

            void operator()(tcp::socket& socket) {
                if (auto connection = connection_.lock()) {
                    ReadMessages(connection->connected_, socket);
                }
                else {
                    LOG_CRITICAL("Connection destructed!");
                }
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                if (auto connection = connection_.lock()) {
                    ReadMessages(connection->ssl_connected_, socket);
                }
                else {
                    LOG_CRITICAL("Connection destructed!");
                }
            }

        private:
            HandlerType handler_;
            std::weak_ptr<Connection> connection_;

            size_t read_buffer_size_;
            std::shared_ptr<std::vector<char>> buffer_ = std::make_shared<std::vector<char>>(read_buffer_size_);

            template <typename Socket>
            void ReadMessages(bool is_connected, Socket& socket) {
                if (auto connection = connection_.lock()) {
                    if (is_connected) {
                        net::async_read(socket, net::buffer(*buffer_)
                            , net::bind_executor(connection->read_strand_
                                , [self = this->shared_from_this()]
                                (const sys::error_code& ec, std::size_t bytes_readed) mutable
                                {
                                    if (bytes_readed > 0) {
                                        self->OnRead(ec, std::vector<char>(self->buffer_->begin(), self->buffer_->begin() + bytes_readed));
                                    }
                                }));
                    }
                    else {
                        throw std::runtime_error("Trying read socket without connection");
                    }
                }
                else {
                    LOG_CRITICAL("Connection destructed!");
                }
            }


            void OnRead(const sys::error_code& ec, std::vector<char>&& bytes) {
                if (ec) {
                    logging::ReportError(ec, "Reading");
                    if (auto con = connection_.lock()) {
                        con->reconnect_required_ = true;
                    }
                }
                if (auto con = connection_.lock()) {
                    net::post(con->read_strand_,
                        [handler = handler_, bytes = std::move(bytes)]() mutable {
                            handler(std::move(bytes));
                        });
                }
            }
        };

        class WriteVisitor : public std::enable_shared_from_this<WriteVisitor> {
        public:
            explicit WriteVisitor(std::shared_ptr<Connection> connection, std::string_view data)
                : connection_(connection)
                , data_(data)
            {

            }

            void operator()(tcp::socket& socket) {
                WriteMessages(connection_->connected_, socket);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                WriteMessages(connection_->ssl_connected_, socket);
            }

        private:
            std::shared_ptr<Connection> connection_;
            std::string data_;

            template <typename Socket>
            void WriteMessages(bool is_connected, Socket& socket) {
                if (!is_connected) {
                    throw std::runtime_error("Writing socket without connection");
                }
                LOG_INFO("Sending: "s.append(data_));
                net::write(socket, net::buffer(data_), connection_->ec_);
            }
        };

        template <typename Handler>
        class AsyncWriteVisitor : public std::enable_shared_from_this<AsyncWriteVisitor<Handler>> {
        public:
            explicit AsyncWriteVisitor(std::shared_ptr<Connection> connection, std::string_view data, Handler&& handler)
                : connection_(connection)
                , data_(data)
                , handler_(std::forward<Handler>(handler))
            {

            }

            void operator()(tcp::socket& socket) {
                AsyncWriteMessages(connection_->connected_, socket);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                AsyncWriteMessages(connection_->ssl_connected_, socket);
            }

        private:
            std::shared_ptr<Connection> connection_;
            std::string data_;
            Handler handler_;

            template <typename Socket>
            void AsyncWriteMessages(bool is_connected, Socket& socket) {
                if (!is_connected) {
                    throw std::runtime_error("Writing socket without connection");
                }
                LOG_INFO("Sending: "s.append(data_));
                net::write(socket, net::buffer(data_), connection_->ec_);

                net::async_write(socket, net::buffer(data_), net::bind_executor(connection_->write_strand_
                    , [self = this->shared_from_this()](const sys::error_code& ec, size_t bytes_writen) {
                        if (ec) {
                            if (ec == boost::asio::error::eof) {
                                LOG_INFO("Connection closed gracefully by server");
                            }
                            else {
                                self->handler_(ec);
                            }
                        }
                    }));
            }
        };

    };

}