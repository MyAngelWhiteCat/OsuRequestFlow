#pragma once

#include "irc_client.h"
#include "auth_data.h"
#include "command_executor.h"
#include "command_parser.h"

#include <memory>
#include <string_view>


namespace core {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class Core {
    public:
        Core(int num_workers)
            : ioc_(num_workers)
        {

        }

        void SetupConnection(bool secured) {
            if (secured) {
                ctx_ = connection::GetSSLContext();
                connection_ = std::make_shared<connection::Connection>
                    (ioc_, *ctx_, read_strand_, write_strand_);
            }
            else {
                connection_ = std::make_shared<connection::Connection>
                    (ioc_, read_strand_, write_strand_);
            }
        }

        void SetupDownloader(bool secured, std::string_view resourse
            , std::string_view uri_prefix, std::string_view downloads_directory) {
            if (secured) {
                ctx_ = connection::GetSSLContext();
                downloader_ = std::make_shared<downloader::Downloader>(ioc_, ctx_);
            }
            else {
                downloader_ = std::make_shared<downloader::Downloader>(ioc_);
            }
            downloader_->SetResourse(resourse);
            downloader_->SetUriPrefix(uri_prefix);
            downloader_->SetDownloadsFolder(downloads_directory);
        }

        void SetupChatBot() {
            if (!downloader_) {
                throw std::logic_error("Setup downloader before setup chat bot");
            }

            command_executor_ = std::make_shared<commands::CommandExecutor>(downloader_);
            command_parser_ = std::make_shared<commands::CommandParser>(*command_executor_);
            chat_bot_ = std::make_shared<chat_bot::ChatBot>(command_executor_, command_parser_);
        }

        void SetAuthData(std::string_view nick, std::string_view tocken) {
            auth_data_.SetNick(nick);
            auth_data_.SetToken(tocken);
        }

        void SetupIRCClient(bool secured) {
            client_ = std::make_shared<irc::Client<chat_bot::ChatBot>>(ioc_, chat_bot_, secured);
        }

        void Start(std::string_view channel_name) {
            CheckReadyness();
            client_->Connect();
            client_->Authorize(auth_data_);
            client_->CapRequest();
            client_->Read();
            client_->Join(channel_name);
        }

        void Run(unsigned num_workers) {
            auto func = [this]() {
                ioc_.run();
                };
            try {
                std::vector<std::jthread> threads;
                for (unsigned i = 0; i < num_workers - 1; ++i) {
                    threads.emplace_back(func);
                }
                func();
            }
            catch (const std::exception& e) {
                LOG_INFO("Catch exception in Run");
                LOG_CRITICAL(e.what());
            }
        }

    private:
        net::io_context ioc_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        Strand read_strand_ = net::make_strand(ioc_);
        Strand write_strand_ = net::make_strand(ioc_);
        Strand connection_strand_ = net::make_strand(ioc_);

        std::shared_ptr<downloader::Downloader> downloader_{ nullptr };
        std::shared_ptr<commands::CommandParser> command_parser_{ nullptr };
        std::shared_ptr<commands::CommandExecutor> command_executor_{ nullptr };
        std::shared_ptr<chat_bot::ChatBot> chat_bot_{ nullptr };
        std::shared_ptr<connection::Connection> connection_{ nullptr };

        std::shared_ptr<irc::Client<chat_bot::ChatBot>> client_{ nullptr };
        irc::domain::AuthorizeData auth_data_;

        void CheckReadyness() {
            if (!downloader_) {
                throw std::logic_error("need to setup downloader");
            }
            if (!command_parser_) {
                throw std::logic_error("need to setup command parser");
            }
            if (!command_executor_) {
                throw std::logic_error("need to setup command executor");
            }
            if (!chat_bot_) {
                throw std::logic_error("need to setup chat bot");
            }
            if (!connection_) {
                throw std::logic_error("need to setup connection");
            }
            if (!client_) {
                throw std::logic_error("need to setup irc client");
            }
        }

    };

} // namespace core