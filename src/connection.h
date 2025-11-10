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


namespace connection {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection(net::io_context& ioc, Strand& write_strand, Strand& read_strand)
            : write_strand_(read_strand)
            , read_strand_(read_strand)
            , socket_(tcp::socket(ioc))
        {

        }

        Connection(net::io_context& ioc, ssl::context& ctx, Strand& write_strand, Strand& read_strand)
            : write_strand_(read_strand)
            , read_strand_(read_strand)
            , socket_(ssl::stream<tcp::socket>(ioc, ctx))
        {

        }

        void Connect(std::string_view host, std::string_view port) {
            ec_.clear();

            ConnectionVisitor visitor(*this, host, port);
            std::visit(visitor, socket_);
            if (ec_) {
                logging::ReportError(ec_, "Connection");
            }
            else {
                address_buffer_ = std::make_pair(std::string(host), std::string(port));
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

        void Reconnect() {
            if (!address_buffer_) {
                throw std::runtime_error("Reconnect buffer empty");
            }
            try {
                Connect(address_buffer_->first, address_buffer_->second);
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
                return;
            }
            reconnected_ = true;
        }

        bool IsReconnected() {
            if (reconnected_) {
                reconnected_ = false;
                return true;
            }
            return false;
        }

        template <typename Handler>
        void Read(Handler&& handler) {
            auto visitor = std::make_shared<ReadVisitor<Handler>>(
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

        bool IsConnected() const {
            return ssl_connected_ || connected_;
        }

    private:
        Strand& write_strand_;
        Strand& read_strand_;
        sys::error_code ec_;
        std::optional<std::pair<std::string, std::string>> address_buffer_;
        std::variant<tcp::socket, ssl::stream<tcp::socket>> socket_;

        bool ssl_connected_ = false;
        bool connected_ = false;
        bool reconnected_ = false;

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
                net::connect(socket, endpoints, connection_.ec_);
                if (connection_.ec_) {
                    logging::ReportError(connection_.ec_, "Connection"sv);
                    return;
                }
                connection_.connected_ = true;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                sys::error_code ec;
                tcp::resolver resolver(socket.get_executor()); // :(
                auto endpoints = resolver.resolve(host_, port_, ec);
                if (ec) {
                    logging::ReportError(ec, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
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
        class ReadVisitor : public std::enable_shared_from_this<ReadVisitor<Handler>> {
        public:

            using HandlerType = typename std::decay<Handler>::type;

            ReadVisitor(const ReadVisitor&) = delete;
            ReadVisitor& operator=(const ReadVisitor&) = delete;

            explicit ReadVisitor(std::weak_ptr<Connection> connection
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
                    LOG_INFO("Reconnecting...");
                    if (auto con = connection_.lock()) {
                        con->Reconnect();
                    }
                }
                handler_(std::move(bytes));
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

        class AsyncWriteVisitor : public std::enable_shared_from_this<AsyncWriteVisitor> {
        public:
            explicit AsyncWriteVisitor(std::shared_ptr<Connection> connection, std::string_view data)
                : connection_(connection)
                , data_(data)
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
                            logging::ReportError(ec, "Writing");
                        }
                    }));
            }
        };
    };

}