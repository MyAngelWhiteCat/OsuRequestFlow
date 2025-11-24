#pragma once

#include "irc_client.h"
#include "auth_data.h"
#include "file_manager.h"
#include "command_executor.h"
#include "command_executor.h"

#include <memory>
#include <string>
#include <string_view>
#include <optional>


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

        void SetupDownloadDirectory(std::string_view path) {
            downloads_file_manager_ = std::make_shared<file_manager::FileManager>
                (std::filesystem::path(path));
        }

        void SetupDownloader(bool secured, std::string_view resourse, std::string_view uri_prefix) {
            if (secured) {
                ctx_ = connection::GetSSLContext();
                downloader_ = std::make_shared<downloader::Downloader>
                    (ioc_, *ctx_, read_strand_, write_strand_);
            }
            else {
                downloader_ = std::make_shared<downloader::Downloader>
                    (ioc_, read_strand_, write_strand_);
            }
            downloader_->SetResourse(resourse);
            downloader_->SetUriPrefix(uri_prefix);
        }

        void SetAuthData(std::string_view nick, std::string_view tocken) {
            auth_data_.SetNick(nick);
            auth_data_.SetToken(tocken);
        }

        void SetupIRCClient(bool secured) {
            client_ = std::make_shared<irc::Client>();
        }

        void ReadChat(std::string_view channel_name) {
            client_->Connect();
            client_->Authorize(auth_data_);
            client_->CapRequest();
            client_->Read();
            client_->Join(channel_name);
        }

    private:
        net::io_context ioc_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        Strand read_strand_ = net::make_strand(ioc_);
        Strand write_strand_ = net::make_strand(ioc_);
        Strand connection_strand_ = net::make_strand(ioc_);

        std::shared_ptr<file_manager::FileManager> downloads_file_manager_{ nullptr };

        std::shared_ptr<downloader::Downloader> downloader_{ nullptr };
        std::shared_ptr<commands::CommandParser> command_parser_{ nullptr };
        std::shared_ptr<commands::CommandExecutor> command_executor_{ nullptr };
        std::shared_ptr<chat_bot::ChatBot> chat_bot_{ nullptr };
        std::shared_ptr<connection::Connection> connection_{ nullptr };
        std::shared_ptr<handler::MessageHandler> message_handler_{ nullptr };

        std::shared_ptr<irc::Client> client_{ nullptr };
        irc::domain::AuthorizeData auth_data_;
    };

} // namespace core