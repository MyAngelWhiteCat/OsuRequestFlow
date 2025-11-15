#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>


#include <memory>
#include <variant>
#include <string_view>
#include <stdexcept>

#include "logging.h"
#include <boost/asio/ssl/stream_base.hpp>
#include <string>
#include <boost/asio/impl/connect.hpp>
#include <utility>


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

    static StringRequest MakeRequest(http::verb method, std::string_view target, int version
        , std::string_view host, std::string_view user_agent, std::string_view accept
        , std::string_view connection) {
        StringRequest req;
        req.method(method);
        req.target(target);
        req.version(version);

        req.set(http::field::host, host);
        req.set(http::field::user_agent, user_agent);
        req.set(http::field::accept, accept);
        req.set(http::field::connection, connection);

        return req;
    }

    class Client : public std::enable_shared_from_this<Client> {
    public:

        Client(net::io_context& ioc, Strand& write_strand, Strand& read_strand)
            : stream_(beast::tcp_stream(ioc))
            , write_strand_(write_strand)
            , read_strand_(read_strand)
        {

        }

        Client(net::io_context& ioc, ssl::context& ctx, Strand& write_strand, Strand& read_strand)
            : stream_(ssl::stream<beast::tcp_stream>(ioc, ctx))
            , write_strand_(write_strand)
            , read_strand_(read_strand)
        {

        }

        ~Client() {
            LOG_INFO("HTTP Client destructed");
        }

        void Connect(std::string_view host, std::string_view port) {
            ec_.clear();

            ConnectionVisitor visitor(*this, host, port);
            std::visit(visitor, stream_);
            if (ec_) {
                logging::ReportError(ec_, "Connection");
            }
        }

        template <typename ResponseHandler>
        void SendRequest(StringRequest&& request, ResponseHandler&& response_handler) {
            auto visitor = std::make_shared<SendVisitor<ResponseHandler>>
                (std::move(request), this->shared_from_this(), std::forward<ResponseHandler>(response_handler));
            std::visit((*visitor), stream_);
        }

        template <typename ResponseHandler>
        void ReadResponse(ResponseHandler&& response_handler) {
            auto visitor = std::make_shared<ReadVisitor<ResponseHandler>>
                (this->shared_from_this(), std::forward<ResponseHandler>(response_handler));
            std::visit((*visitor), stream_);
        }

    private:
        std::variant<beast::tcp_stream, ssl::stream<beast::tcp_stream>> stream_;
        Strand write_strand_;
        Strand read_strand_;
        beast::error_code ec_;
        bool ssl_connected_ = false;
        bool connected_ = false;


        template <typename Handler>
        class ReadVisitor : public std::enable_shared_from_this<ReadVisitor<Handler>> {
        public:
            ReadVisitor(std::shared_ptr<Client> client, Handler&& handler)
                : client_(client)
                , handler_(std::forward<Handler>(handler))
            {

            }

            ~ReadVisitor() {
                LOG_INFO("ReadVisitor destructed");
            }

            void operator()(beast::tcp_stream& stream) {
                Read(stream);
            }

            void operator()(ssl::stream<beast::tcp_stream>& stream) {
                Read(stream);
            }

        private:
            Handler&& handler_;
            std::shared_ptr<Client> client_;
            beast::flat_buffer buffer_;
            DynamicResponse dynamic_response_;

            template <typename Stream>
            void Read(Stream& stream) {
                LOG_INFO("Start reading response");
                http::async_read(stream, buffer_, dynamic_response_
                    , net::bind_executor(client_->read_strand_
                    , [self = this->shared_from_this()]
                    (const beast::error_code& ec, size_t bytes_readed) mutable
                        {
                            LOG_INFO("Read "s.append(std::to_string(bytes_readed).append(" bytes")));
                            self->OnRead(ec);
                        }));
            }

            void OnRead(const beast::error_code& ec) {
                if (ec) {
                    logging::ReportError(ec, "Reading http response");
                    return;
                }
                handler_(std::move(dynamic_response_));
            }

        };

        template <typename Handler>
        class SendVisitor : public std::enable_shared_from_this<SendVisitor<Handler>> {
        public:
            SendVisitor(StringRequest&& request, std::shared_ptr<Client> client, Handler&& handler)
                : client_(client)
                , handler_(std::forward<Handler>(handler))
                , request_(std::move(request))
            {

            }

            void operator()(beast::tcp_stream& stream) {
                Send(stream);
            }

            void operator()(ssl::stream<beast::tcp_stream>& stream) {
                Send(stream);
            }
        
        private:
            Handler&& handler_;
            std::shared_ptr<Client> client_;
            StringRequest request_;

            template <typename Stream>
            void Send(Stream& stream) {
                http::async_write(stream, request_
                    , net::bind_executor(client_->write_strand_, [self = this->shared_from_this()]
                    (const beast::error_code& ec, size_t bytes_writen)
                    {
                        LOG_INFO("Send "s.append(std::to_string(bytes_writen).append(" bytes")));
                        self->OnSend(ec);
                    }));
            }

            void OnSend(const beast::error_code& ec) {
                if (ec) {
                    logging::ReportError(ec, "Send request");
                    return;
                }
                client_->ReadResponse(std::forward<Handler>(handler_));
            }

        };

        class ConnectionVisitor {
        public:
            explicit ConnectionVisitor(Client& client, std::string_view host, std::string_view port)
                : client_(client)
                , host_(host)
                , port_(port)
            {
            }

            void operator()(beast::tcp_stream& socket) {
                tcp::resolver resolver(socket.get_executor()); // :(
                auto endpoints = resolver.resolve(host_, port_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                    for (const auto& ep : endpoints) {
                        LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                    }
                }

                socket.connect(endpoints, client_.ec_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "Connection"sv);
                    return;
                }
                client_.connected_ = true;
                LOG_INFO("CONNECTED");
            }

            void operator()(ssl::stream<beast::tcp_stream>& socket) {
                tcp::resolver resolver(socket.get_executor()); // :(
                auto endpoints = resolver.resolve(host_, port_, client_.ec_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "Resolving");
                }
                else {
                    LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                    for (const auto& ep : endpoints) {
                        LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                    }
                }
                net::connect(socket.lowest_layer(), endpoints, client_.ec_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "SSL Connection"sv);
                    return;
                }
                else {
                    LOG_INFO("CONNECTED! HANDSHAKE REQUIRED...");
                }
                socket.lowest_layer().set_option(tcp::no_delay(true));
                socket.handshake(ssl::stream_base::client, client_.ec_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "SSL Handshake"sv);
                    socket.lowest_layer().close();
                    return;
                }
                else {
                    LOG_INFO("HANDSHAKE SUCESS");
                }
                client_.ssl_connected_ = true;
                LOG_INFO("SSL CONNECTED");
            }

        private:
            Client& client_;
            std::string host_;
            std::string port_;
        };
    };

}