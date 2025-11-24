#pragma once

#ifdef WIN32
#include <sdkddkver.h>
#endif


#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/impl/context.ipp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <openssl/ssl.h>

#include <string>
#include <string_view>
#include <variant>
#include <memory>
#include <stdexcept>

#include "logging.h"
#include "ca_sertificates_loader.h"


namespace connection {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    // AI on
    static std::shared_ptr<ssl::context> GetSSLContext() {
        auto ctx = std::make_shared<ssl::context>(ssl::context::tls_client);

        SSL_CTX_set_info_callback(ctx->native_handle(), [](const SSL* ssl, int where, int ret) {
            if (where & SSL_CB_HANDSHAKE_START) {
                LOG_INFO("SSL Handshake starting...");
            }
            if (where & SSL_CB_HANDSHAKE_DONE) {
                LOG_INFO("SSL Handshake completed!");
            }
            });

        ctx->set_options(
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 |
            ssl::context::no_sslv3 |
            ssl::context::no_tlsv1 |
            ssl::context::no_tlsv1_1
        );

        ctx->set_verify_mode(ssl::verify_peer);

        try {
            ctx->set_default_verify_paths();
            LOG_INFO("Default verify paths set successfully");
        }
        catch (const std::exception& e) {
            LOG_ERROR("set_default_verify_paths failed: "s.append(e.what()));
        }

        ssl_domain_utilities::load_windows_ca_certificates(*ctx);

        const char* ciphers =
            "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:!aNULL:!MD5:!DSS:!RC4";

        if (SSL_CTX_set_cipher_list(ctx->native_handle(), ciphers) != 1) {
            LOG_ERROR("Failed to set cipher list");
        }

        SSL_CTX_set_min_proto_version(ctx->native_handle(), TLS1_2_VERSION);

        return ctx;
    }
    // AI OFF

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection(net::io_context& ioc, Strand& read_strand, Strand& write_strand);

        Connection(net::io_context& ioc, ssl::context& ctx, Strand& read_strand, Strand& write_strand);

        void Connect(std::string_view host, std::string_view port);

        void Disconnect(bool is_need_to_close_socket = true);

        bool IsReconnectRequired();

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

        bool IsConnected() const;

        net::io_context* GetContext();

        bool IsSecured() const;

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

            void operator()(tcp::socket& socket);

            void operator()(ssl::stream<tcp::socket>& socket);

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

            void operator()(tcp::socket& socket);

            void operator()(ssl::stream<tcp::socket>& socket);

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
                , size_t read_buffer_size = 128)
                : connection_(connection)
                , handler_(std::forward<Handler>(handler))
                , read_buffer_size_(read_buffer_size)
            {
            }

            void operator()(tcp::socket& socket) {
                ReadMessages(connection_->connected_, socket);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                ReadMessages(connection_->ssl_connected_, socket);
            }

        private:
            HandlerType handler_;
            std::shared_ptr<Connection> connection_;

            size_t read_buffer_size_;
            std::shared_ptr<std::vector<char>> buffer_ = std::make_shared<std::vector<char>>(read_buffer_size_);

            template <typename Socket>
            void ReadMessages(bool is_connected, Socket& socket) {
                if (is_connected) {
                    net::async_read(socket, net::buffer(*buffer_), net::transfer_at_least(1)
                        , net::bind_executor(connection_->read_strand_
                            , [self = this->shared_from_this()]
                            (const sys::error_code& ec, std::size_t bytes_readed) mutable
                            {
                                if (bytes_readed > 0) {
                                    self->OnRead(ec, std::vector<char>(self->buffer_->begin(), self->buffer_->begin() + bytes_readed));
                                }
                                else {
                                    LOG_ERROR("0 bytes readed");
                                }
                            }));
                }
                else {
                    throw std::runtime_error("Trying read socket without connection");
                }
            }


            void OnRead(const sys::error_code& ec, std::vector<char>&& bytes) {
                if (ec) {
                    logging::ReportError(ec, "Reading");
                    connection_->reconnect_required_ = true;
                }
                net::post(connection_->read_strand_,
                    [handler = handler_, bytes = std::move(bytes)]() mutable {
                        handler(std::move(bytes));
                    });
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