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
#include <unordered_map>


#include "logging.h"
#include <boost/asio/ssl/stream_base.hpp>
#include <string>
#include <boost/asio/impl/connect.hpp>
#include <utility>
#include <optional>
#include <fstream>


namespace http_domain {

    constexpr size_t KiB = 1024;
    constexpr size_t MiB = 1024 * KiB;
    constexpr size_t GiB = 1024 * MiB;
    constexpr size_t MAX_FILE_SIZE = 20 * MiB;


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

    struct Port {
        static constexpr std::string_view SECURED = "443"sv;
        static constexpr std::string_view NON_SECURED = "80"sv;
    };

    struct Fields {
        static constexpr std::string_view CONTENT_LENGTH = "Content-Length";
        static constexpr std::string_view CONTENT_DISPOSITION = "Content-Disposition";
    };

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

        Client(net::io_context& ioc)
            : stream_(beast::tcp_stream(ioc))
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
        {

        }

        Client(net::io_context& ioc, ssl::context& ctx)
            : stream_(ssl::stream<beast::tcp_stream>(ioc, ctx))
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
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
            else {
                host_ = host;
            }
        }

        template <typename ResponseHandler>
        void SendRequest(StringRequest&& request, ResponseHandler&& response_handler) {
            busy_ = true;
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

        bool IsConnected() const {
            return ssl_connected_ || connected_;
        }

        bool IsBusy() const {
            return busy_;
        }

        template <typename ResponseHandler>
        void Get(std::string_view target, std::string_view user_agent, ResponseHandler&& handler) {
            LOG_INFO("Get");
            if (!IsConnected() || !host_) {
                throw std::logic_error("Trying send request without connection");
            }
            auto req = MakeRequest(http::verb::get, target, 11, *host_, user_agent, accept_, connection_);
            req.set(http::field::accept_encoding, "gzip, deflate, br");
            SendRequest(std::move(req), std::forward<ResponseHandler>(handler));
        }

        template <typename ResponseHandler>
        void Head(std::string_view target, std::string_view user_agent, ResponseHandler&& handler) {
            if (!IsConnected() || !host_) {
                throw std::logic_error("Trying send request without connection");
            }
            auto req = MakeRequest(http::verb::head, target, 11, *host_, user_agent, accept_, connection_);
            SendRequest(std::move(req), std::forward<ResponseHandler>(handler));
        }

    private:
        std::variant<beast::tcp_stream, ssl::stream<beast::tcp_stream>> stream_;
        Strand write_strand_;
        Strand read_strand_;
        beast::error_code ec_;
        bool ssl_connected_ = false;
        bool connected_ = false;
        bool busy_ = false;
        std::optional<std::string> host_;
        std::string accept_ = "*/*";
        std::string connection_ = "keep-alive";

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
            std::shared_ptr<Client> client_;
            Handler handler_;

            beast::flat_buffer buffer_;
            DynamicResponse dynamic_response_;
            http::response_parser<http::dynamic_body> parser_;

            std::unordered_map<std::string, std::string> header_to_value_;
            std::vector<char> body_bytes_;

            template <typename Stream>
            void Read(Stream& stream) {
                ReadHeaders(stream);
            }

            template <typename Stream>
            void ReadHeaders(Stream& stream) {
                LOG_INFO("Start reading response");
                http::async_read_header(stream, buffer_, parser_
                    , net::bind_executor(client_->read_strand_
                        , [self = this->shared_from_this(), &stream]
                        (const beast::error_code& ec, size_t bytes_readed) mutable
                        {
                            LOG_INFO("Read "s.append(std::to_string(bytes_readed).append(" bytes")));
                            LOG_INFO("Headers readed");
                            self->OnReadHeaders(ec, stream);
                        }));
            }

            template <typename Stream>
            void OnReadHeaders(const beast::error_code& ec, Stream& stream) {
                if (ec) {
                    CheckConnectionError(ec);
                    logging::ReportError(ec, "Reading http response");
                    return;
                }
                
                PrintResponseHeaders();
                LOG_INFO(GetFileName());
                std::ofstream out(GetFileName());
                out << "mic check";
                if (CheckFileSize()) {
                    //ReadBody(stream);
                }
            }

            void PrintResponseHeaders() {
                auto& headers = parser_.get();
                for (const auto& header : headers) {
                    std::cout << header.name_string() << ": " << header.value() << "\n";
                }
            }

            bool CheckFileSize() {
                auto& headers = parser_.get();
                auto it = headers.find(Fields::CONTENT_LENGTH);
                if (it != headers.end()) {
                    return std::stoll(it->value()) < MAX_FILE_SIZE;
                }
                return false;
            }

            std::string GetFileName() {
                auto& headers = parser_.get();
                auto it = headers.find(Fields::CONTENT_DISPOSITION);
                if (it != headers.end()) {
                    return ParseFileName(it->value());
                }
                return "JohnDoe";
            }

            std::string ParseFileName(std::string_view content) {
                size_t start_pos = content.find_first_of('"') + 1;
                size_t end_pos = content.find_last_of('"');
                if (start_pos != std::string::npos && end_pos != std::string::npos) {
                    return std::string(content.substr(start_pos, end_pos - start_pos));
                }
                return "JohnDoe";
            }

            template <typename Stream>
            void ReadBody(Stream& stream) {
                http::async_read(stream, buffer_, parser_
                    , [self = shared_from_this(), &stream](const beast::error_code& ec, size_t bytes_readed) mutable {
                        LOG_INFO("Read "s.append(std::to_string(bytes_readed).append(" bytes")));
                        self->OnBodyRead(ec);
                    });
            }

            template <typename Stream>
            void OnReadBody(const beast::error_code& ec, Stream& stream) {
                if (ec) {
                    CheckConnectionError(ec);
                    logging::ReportError(ec, "Reading http response");
                    if (ec == http::error::need_more) {
                        auto moved_chank(std::make_move_iterator(body_bytes_.begin())
                                       , std::make_move_iterator(body_bytes_.end()));
                        handler_(header_to_value_, std::move(moved_chank), false);
                        ReadFileStream(stream);
                    }
                }
                dynamic_response_ = parser_.release();
                ParseResponseBody();
                LOG_INFO("Body readed");
                client_->busy_ = false;
                handler_(std::move(header_to_value_), std::move(body_bytes_));
            }

            template <typename Stream>
            void ReadFileStream(Stream& stream) {
                dynamic_response_.clear();


            }

            void ParseResponseBody() {
                body_bytes_.clear();
                auto buffers = dynamic_response_.body().data();
                for (auto it = buffers.begin(); it != buffers.end(); ++it) {
                    auto buffer = *it;
                    const char* data = static_cast<const char*>(buffer.data());
                    body_bytes_.insert(body_bytes_.end(), data, data + buffer.size());
                }
            }

            void CheckConnectionError(const beast::error_code& ec) {
                if (ec == net::error::eof ||
                    ec == net::error::connection_reset ||
                    ec == beast::http::error::end_of_stream) {
                    client_->connected_ = false;
                    client_->ssl_connected_ = false;
                }
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
            Handler handler_;
            std::shared_ptr<Client> client_;
            StringRequest request_;

            template <typename Stream>
            void Send(Stream& stream) {
                std::cout << "ACTUAL REQUEST:\n" << request_ << "\n";

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
                    if (ec == net::error::eof ||
                        ec == net::error::connection_reset ||
                        ec == beast::http::error::end_of_stream) {
                        client_->connected_ = false;
                        client_->ssl_connected_ = false;
                    }
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
                    throw std::runtime_error("cant resolve: "s.append(host_).append(" ").append(port_));
                }

                LOG_INFO("Resolved "s.append(host_).append(":"s).append(port_));
                for (const auto& ep : endpoints) {
                    LOG_INFO(endpoints.begin()->endpoint().address().to_string().append(":"s).append(port_));
                }

                SSL_set_tlsext_host_name(socket.native_handle(), host_.c_str());
                net::connect(socket.lowest_layer(), endpoints, client_.ec_);
                if (client_.ec_) {
                    logging::ReportError(client_.ec_, "SSL Connection"sv);
                    return;
                }
                else {
                    LOG_INFO("CONNECTED");
                }
                socket.lowest_layer().set_option(tcp::no_delay(true));
                socket.handshake(ssl::stream_base::client, client_.ec_);
                if (client_.ec_) {
                    ERR_print_errors_fp(stderr);

                    logging::ReportError(client_.ec_, "SSL Handshake");
                    socket.lowest_layer().close();
                    return;
                }

                else {
                    LOG_INFO("HANDSHAKE SUCESS");
                }
                client_.ssl_connected_ = true;
                if (SSL_get_verify_result(socket.native_handle()) != X509_V_OK) {
                    LOG_INFO("SSL Certificate verification failed");
                }
                else {
                    LOG_INFO("SSL Certificate verified successfully");
                }
            }

        private:
            Client& client_;
            std::string host_;
            std::string port_;
        };
    };

}