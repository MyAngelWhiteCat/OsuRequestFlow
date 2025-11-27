#pragma once

#include "irc_client.h"
#include "auth_data.h"
#include "command_executor.h"
#include "command_parser.h"
#include "nlohmann/json.hpp"
#include "user_validator.h"

#include <memory>
#include <string_view>
#include <fstream>


namespace core {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using json = nlohmann::json;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    struct SettingsKeys {
        static constexpr std::string_view FILENAME = "settings.json"sv;
        static constexpr std::string_view USER_VERIFICATOR = "user_verificator"sv;
        static constexpr std::string_view WHITELIST = "whitelist"sv;
        static constexpr std::string_view BLACKLIST = "blacklist"sv;
        static constexpr std::string_view WHITELIST_ONLY = "whitelist_only"sv;
        static constexpr std::string_view ROLEFILTER_LEVEL = "rolefilter_level"sv;
    };

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

        void Start() {
            CheckReadyness();
            client_->Connect();
            client_->Authorize(auth_data_);
            client_->CapRequest();
            client_->Read();
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

        void SaveSettings() {
            json settings;
            settings[SettingsKeys::USER_VERIFICATOR] = GetUserVerificatorSettings();
            std::ofstream out(std::string(SettingsKeys::FILENAME.data(), SettingsKeys::FILENAME.size()));
            out << settings.dump(4);
        }

        void LoadSettings() {
            json settings;
            std::ifstream in(std::string(SettingsKeys::FILENAME.data(), SettingsKeys::FILENAME.size()));
            in >> settings;
            std::cout << settings << std::endl;
            LoadUserVerificator(settings.at(SettingsKeys::USER_VERIFICATOR));
        }

        // user_validator

        void AddUserInWhiteList(std::string_view user) {
           command_executor_->GetUserVerificator()->AddUserInWhiteList(user);
        }

        void AddUserInBlackList(std::string_view user) {
            command_executor_->GetUserVerificator()->AddUserInBlackList(user);
        }

        void RemoveUserFromWhiteList(std::string_view user) {
            command_executor_->GetUserVerificator()->RemoveUserFromWhiteList(user);
        }

        void RemoveUserFromBlackList(std::string_view user) {
            command_executor_->GetUserVerificator()->RemoveUserFromBlackList(user);
        }

        void SetRoleLevelFilter(int level) {
            command_executor_->GetUserVerificator()->SetRoleLevel(level);
        }

        void SetWhiteListOnly(bool on) {
            command_executor_->GetUserVerificator()->SetWhiteListOnly(on);
        }

        // downloader

        void SetDownloadResourseAndPrefix(std::string_view resourse, std::string_view prefix) {
            downloader_->SetResourse(resourse);
            downloader_->SetUriPrefix(prefix);
        }

        void SetDownloadsFolder(std::string_view path) {
            downloader_->SetDownloadsFolder(path);
        }

        void SetMaxFileSize(int MiB) {
            downloader_->SetMaxFileSize(MiB);
        }

        // irc client

        void Join(std::string_view channel) {
            client_->Join(channel);
        }

        void Part(std::string_view channel) {
            client_->Part(channel);
        }

        void SetReconnectTimeout(int seconds) {
            client_->SetReconnectTimeout(seconds);
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

        json GetUserVerificatorSettings() {
            json settings;
            auto* const user_verificator = command_executor_->GetUserVerificator();
            const auto* const black_list = user_verificator->GetBlackList();
            const auto* const white_list = user_verificator->GetWhiteList();
            settings[SettingsKeys::BLACKLIST] = std::vector<std::string>
                (black_list->begin(), black_list->end());
            settings[SettingsKeys::WHITELIST] = std::vector<std::string>
                (white_list->begin(), white_list->end());
            settings[SettingsKeys::ROLEFILTER_LEVEL] = user_verificator->GetRoleLevel();
            settings[SettingsKeys::WHITELIST_ONLY] = user_verificator->GetWhiteListOnly();
            return settings;
        }

        void LoadUserVerificator(json& settings) {
            auto black_list = settings.at(SettingsKeys::BLACKLIST).get<std::vector<std::string>>();
            auto white_list = settings.at(SettingsKeys::WHITELIST).get<std::vector<std::string>>();
            int role_level = settings.at(SettingsKeys::ROLEFILTER_LEVEL).get<int>();
            bool is_whitelist_only = settings.at(SettingsKeys::WHITELIST_ONLY).get<bool>();
            commands::user_validator::UserVerificator verificator(white_list, black_list);
            command_executor_->SetUserVerificator(std::move(verificator));
        }

    };

} // namespace core