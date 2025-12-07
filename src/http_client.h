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
#include <utility>
#include <optional>
#include <fstream>
#include <iomanip>

#include "logging.h"
#include "request_builder.h"
#include "response_parser.h"


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

    struct Port {
        static constexpr std::string_view SECURED = "443"sv;
        static constexpr std::string_view NON_SECURED = "80"sv;
    };

    class Client : public std::enable_shared_from_this<Client> {
    public:

        Client(net::io_context& ioc)
            : stream_(beast::tcp_stream(ioc))
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
        {

        }

        Client(net::io_context& ioc, std::shared_ptr<ssl::context> ctx)
            : ctx_(ctx)
            , stream_(ssl::stream<beast::tcp_stream>(ioc, *ctx))
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
        {

        }

        void SetMaxFileSize(int file_size_MiB) {
            max_file_size_MiB_ = file_size_MiB;
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

        template <typename ResponseHandler>
        void Get(std::string_view target, std::string_view user_agent, ResponseHandler&& handler) {
            if (!IsConnected() || !host_) {
                throw std::logic_error("Trying send request without connection");
            }
            auto req = request_builder_.Get(target, user_agent, *host_);
            SendRequest(std::move(req), std::forward<ResponseHandler>(handler));
        }

        template <typename ResponseHandler>
        void Head(std::string_view target, std::string_view user_agent, ResponseHandler&& handler) {
            if (!IsConnected() || !host_) {
                throw std::logic_error("Trying send request without connection");
            }
            auto req = request_builder_.Head(target, user_agent, *host_);
            SendRequest(std::move(req), std::forward<ResponseHandler>(handler));
        }

        void SetRootDirectory(const std::filesystem::path& path) {
            root_directory_ = path;
        }

    private:
        std::variant<beast::tcp_stream, ssl::stream<beast::tcp_stream>> stream_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };

        Strand write_strand_;
        Strand read_strand_;

        beast::error_code ec_;

        bool ssl_connected_ = false;
        bool connected_ = false;

        std::optional<std::string> host_;

        RequestBuilder request_builder_;
        int max_file_size_MiB_ = 200;

        std::optional<std::filesystem::path> root_directory_;


        template <typename Handler>
        class ReadVisitor : public std::enable_shared_from_this<ReadVisitor<Handler>> {
        public:
            ReadVisitor(std::shared_ptr<Client> client, Handler&& handler)
                : client_(client)
                , handler_(std::forward<Handler>(handler))
                , response_parser_(*client_->root_directory_, client->max_file_size_MiB_)
            {
                // FATAL
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
            FileResponseParser response_parser_;
            bool in_downloading_ = false;
            net::steady_timer downloading_monitoring_timer_{client_->write_strand_.get_inner_executor().context()};
            std::vector<char> downloads_fasle_;
            double last_knowed_progress_ = 0;

            template <typename Stream>
            void Read(Stream& stream) {
                ReadHeaders(stream);
            }

            template <typename Stream>
            void ReadHeaders(Stream& stream) {
                http::async_read_header(stream, buffer_, response_parser_.GetParser()
                    , net::bind_executor(client_->read_strand_
                        , [self = this->shared_from_this(), &stream]
                        (const beast::error_code& ec, size_t bytes_readed) mutable
                        {
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
                response_parser_.PrintResponseHeaders();
                if (!response_parser_.IsOK()) {
                    LOG_ERROR("Response status is not OK");
                    return;
                }

                response_parser_.CheckRedirect();
                if (response_parser_.CheckFileSize()) {
                    ReadBody(stream);
                }
                else {
                    LOG_ERROR("File size too big");
                }
            }

            template <typename Stream>
            void ReadBody(Stream& stream) {
                in_downloading_ = true;
                if (!response_parser_.OpenFile()) {
                    LOG_INFO("Cant open file for download: "s.append(std::string(response_parser_.GetFileName())));
                }
                http::async_read(stream, buffer_, response_parser_.GetParser()
                    , [self = this->shared_from_this(), &stream](const beast::error_code& ec, size_t bytes_readed) mutable {
                        self->in_downloading_ = true;
                        LOG_INFO("Read "s.append(std::to_string(bytes_readed).append(" bytes")));
                        self->OnReadBody(ec, bytes_readed);
                    });
                LOG_INFO("Start downloading");
                MonitorDownloading(stream);
            }

            void OnReadBody(const beast::error_code& ec, size_t bytes_readed) {
                if (ec) {
                    if (ec == net::error::operation_aborted) {
                        LOG_ERROR("Download cancelled.");
                        return;
                    }
                    CheckConnectionError(ec);
                    logging::ReportError(ec, "Reading http response");
                }
                auto file_name = response_parser_.GetFileName();
                LOG_INFO("Body readed");
                handler_(std::move(std::string(file_name)), bytes_readed);
            }

            void CheckConnectionError(const beast::error_code& ec) {
                if (ec == net::error::eof ||
                    ec == net::error::connection_reset ||
                    ec == beast::http::error::end_of_stream) {
                    client_->connected_ = false;
                    client_->ssl_connected_ = false;
                }
            }

            template <typename Stream>
            void MonitorDownloading(Stream& stream) {
                downloading_monitoring_timer_.expires_after(std::chrono::milliseconds(50));
                downloading_monitoring_timer_.async_wait([self = this->weak_from_this(), &stream](auto ec) {
                    if (auto cli = self.lock(); !ec && cli->in_downloading_) {
                        double progress = cli->response_parser_.GetProgress();
                        if (progress != cli->last_knowed_progress_) {
                            std::cout << std::setprecision(2) << progress << "%\n";
                        }

                        if (progress == cli->last_knowed_progress_) {
                            cli->downloads_fasle_.push_back('c');
                            if (cli->downloads_fasle_.size() == 200) {
                                auto& lowest_layer = beast::get_lowest_layer(stream);
                                lowest_layer.cancel();
                            }

                        }
                        else {
                            if (!cli->downloads_fasle_.empty()) { cli->downloads_fasle_.pop_back(); }
                        }
                        cli->last_knowed_progress_ = progress;
                        cli->MonitorDownloading(stream);
                    }
                    });
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
                std::ofstream log_request("LogRequest.txt", std::ios::app);
                log_request << "REQUEST:\n" << request_ << "\n\n";

                http::async_write(stream, request_
                    , net::bind_executor(client_->write_strand_, [self = this->shared_from_this()]
                    (const beast::error_code& ec, size_t bytes_writen)
                        {
                            self->OnSend(ec);
                        }));
            }

            void OnSend(const beast::error_code& ec) {
                if (ec) {
                    CheckConnectionError(ec);
                    logging::ReportError(ec, "Send request");
                    return;
                }
                client_->ReadResponse(std::forward<Handler>(handler_));
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
                tcp::resolver resolver(socket.get_executor());
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
                    LOG_ERROR("SSL Certificate verification failed");
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
