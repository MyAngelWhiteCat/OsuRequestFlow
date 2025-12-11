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
#include "connection_pool.h"


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

    struct DLMetaData {
        std::string file_name_;
        size_t file_size_;
        double speed_mbs_;
        bool success = true;
    };

    class Client : public std::enable_shared_from_this<Client> {
    public:

        Client(net::io_context& ioc)
            : connection_(ioc)
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
        {

        }

        Client(net::io_context& ioc, std::shared_ptr<ssl::context> ctx)
            : connection_(ioc, ctx)
            , write_strand_(net::make_strand(ioc))
            , read_strand_(net::make_strand(ioc))
        {

        }

        void Connect(std::string_view host, std::string_view port, int attempt = 1) {
            const int attempts = 2;
            const int timeout_sec = 5;

            if (attempt > attempts) {
                LOG_ERROR("Cant connect to "s.append(std::string(host).append(":"s).append(std::string(port))));
                return;
            }

            if (connection_.Connect(host, port)) {
                host_ = std::string(host);
            }
            else {
                LOG_INFO("Reconnecting attempt "s.append(std::to_string(attempt)
                    .append(" / ").append(std::to_string(attempts))));
                std::this_thread::sleep_for(std::chrono::seconds(timeout_sec));
                Connect(host, port, ++attempt);
            }
        }

        void SetMaxFileSize(int file_size_MiB) {
            max_file_size_MiB_ = file_size_MiB;
        }

        template <typename ResponseHandler>
        void SendRequest(StringRequest&& request, ResponseHandler&& response_handler) {
            auto visitor = std::make_shared<SendVisitor<ResponseHandler>>
                (std::move(request), this->shared_from_this(), std::forward<ResponseHandler>(response_handler));
            std::visit((*visitor), connection_.GetStream());
        }

        template <typename ResponseHandler>
        void ReadResponse(ResponseHandler&& response_handler) {
            if (speed_mesure_mode_) {
                auto visitor = std::make_shared<ReadVisitor<ResponseHandler
                    , http::response_parser<http::dynamic_body>>>
                    (this->shared_from_this(), std::forward<ResponseHandler>(response_handler));
                visitor->SetSpeedMesureMode(speed_mesure_mode_);
                std::visit((*visitor), connection_.GetStream());
                return;
            }
            auto visitor = std::make_shared<ReadVisitor<ResponseHandler
                , http::response_parser<http::file_body>>>
                (this->shared_from_this(), std::forward<ResponseHandler>(response_handler));
            std::visit((*visitor), connection_.GetStream());
        }

        bool IsConnected() const {
            return connection_.IsConnected();
        }

        template <typename ResponseHandler>
        void Get(std::string_view target, std::string_view user_agent, ResponseHandler&& handler) {
            if (!IsConnected() || !host_) {
                throw std::logic_error("Trying send request without connection");
            }
            auto req = request_builder_.Get(target, user_agent, *host_);
            SendRequest(std::move(req), std::forward<ResponseHandler>(handler));
        }

        void SetRootDirectory(const std::filesystem::path& path) {
            root_directory_ = path;
        }

        void SetSpeedMesureMode(bool is_speed_mesuring) {
            speed_mesure_mode_ = is_speed_mesuring;
        }

    private:
        Connection connection_;
        Strand write_strand_;
        Strand read_strand_;
        std::optional<std::string> host_;

        beast::error_code ec_;
        RequestBuilder request_builder_;
        int max_file_size_MiB_ = 200;
        std::optional<std::filesystem::path> root_directory_;

        bool speed_mesure_mode_ = false;

        template <typename Handler, typename Parser>
        class ReadVisitor : public std::enable_shared_from_this<ReadVisitor<Handler, Parser>> {
        public:
            ReadVisitor(std::shared_ptr<Client> client, Handler&& handler)
                : client_(client)
                , handler_(std::forward<Handler>(handler))
                , response_parser_(client->max_file_size_MiB_)
            {
                if (client->root_directory_) {
                    response_parser_.SetRootDirectory(*client->root_directory_);
                }
                else {
                    throw std::runtime_error("Need to set dowloader folder");
                }
            }

            void operator()(beast::tcp_stream& stream) {
                Read(stream);
            }

            void operator()(ssl::stream<beast::tcp_stream>& stream) {
                Read(stream);
            }

            void SetSpeedMesureMode(bool is_speed_mesuring) {
                response_parser_.SetSpeedMesureMode(is_speed_mesuring);
            }

        private:
            std::shared_ptr<Client> client_;
            Handler handler_;

            std::chrono::steady_clock::time_point download_start_;
            std::chrono::steady_clock::time_point download_end_;

            beast::flat_buffer buffer_;
            FileResponseParser<Parser> response_parser_;
            bool in_downloading_ = false;
            bool speed_mesure_mode_ = false;

            net::steady_timer downloading_monitoring_timer_{ client_->write_strand_.get_inner_executor().context() };
            int no_progress_streak_ = 0;
            double last_knowed_progress_ = 0;

            template <typename Stream>
            void Read(Stream& stream) {
                ReadHeaders(stream);
            }

            template <typename Stream>
            void ReadHeaders(Stream& stream) {
                http::async_read_header(stream, buffer_
                    ,  response_parser_.GetParser()
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
                if (!speed_mesure_mode_ && !response_parser_.OpenFile()) {
                    LOG_ERROR("Cant open file for download: "s.append(std::string(response_parser_.GetFileName())));
                    return;
                }
                in_downloading_ = true;
                download_start_ = std::chrono::steady_clock::now();
                http::async_read(stream, buffer_, response_parser_.GetParser()
                    , [self = this->shared_from_this(), &stream](const beast::error_code& ec, size_t bytes_readed) mutable {
                        self->download_end_ = std::chrono::steady_clock::now();
                        self->in_downloading_ = false;
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
                        DLMetaData metadata;
                        metadata.success = false;
                        handler_(std::move(metadata));
                        return;
                    }
                    CheckConnectionError(ec);
                    logging::ReportError(ec, "Reading http response");
                }
                LOG_INFO("Body readed");

                double time_spend = std::chrono::duration_cast<std::chrono::milliseconds>
                    (download_end_ - download_start_).count();

                const int MILLISECONDS_IN_SECOND = 1000;
                DLMetaData metadata;
                metadata.file_name_ = std::string(response_parser_.GetFileName());
                metadata.file_size_ = bytes_readed;
                metadata.speed_mbs_ = static_cast<double>(bytes_readed * MiB) 
                    / (time_spend * MILLISECONDS_IN_SECOND);
                handler_(std::move(metadata));
            }

            void CheckConnectionError(const beast::error_code& ec) {
                if (ec == net::error::eof ||
                    ec == net::error::connection_reset ||
                    ec == beast::http::error::end_of_stream) {
                    client_->connection_.ConnectionReset();
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
                            ++cli->no_progress_streak_;

                            if (cli->no_progress_streak_ >= 200) {
                                auto& lowest_layer = beast::get_lowest_layer(stream);
                                lowest_layer.cancel();
                            }
                        }
                        else {
                            if (cli->no_progress_streak_ > 0) { 
                                --cli->no_progress_streak_; 
                            }
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
                    client_->connection_.ConnectionReset();
                }
            }

        };

    };

}
