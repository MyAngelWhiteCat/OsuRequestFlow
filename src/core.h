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
#include <utility>


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
        Core(net::io_context& ioc)
            : ioc_(ioc)
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
            downloader_ = std::make_shared<downloader::Downloader>(ioc_, secured);
            downloader_->SetResourse(resourse);
            downloader_->SetUriPrefix(uri_prefix);
            downloader_->SetDownloadsFolder(downloads_directory);
            SetupChatBot();
        }

        void SetupDownloader(bool secured) {
            downloader_ = std::make_shared<downloader::Downloader>(ioc_, secured);
            SetupChatBot();
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
            //CheckReadyness();
            client_->Connect();
            client_->Authorize(auth_data_);
            client_->CapRequest();
            client_->Read();
        }

        // settings

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

        std::unordered_set<std::string>* GetWhiteList() {
            return command_executor_->GetUserVerificator()->GetWhiteList();
        }

        std::unordered_set<std::string>* GetBlackList() {
            return command_executor_->GetUserVerificator()->GetBlackList();
        }

        bool IsUserInWhiteList(std::string_view username) {
            return command_executor_->GetUserVerificator()->GetWhiteList()->count(std::string(username));
        }

        bool IsUserInBlackList(std::string_view username) {
            return command_executor_->GetUserVerificator()->GetBlackList()->count(std::string(username));
        }

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

        void SetMaxFileSize(size_t MiB) {
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
        net::io_context& ioc_;
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
            std::vector<std::string> white_list;
            std::vector<std::string> black_list;
            if (auto it = settings.find(SettingsKeys::BLACKLIST); it != settings.end()) {
                auto black_list_ = it->get<std::vector<std::string>>();
                black_list.reserve(black_list_.size());
                black_list = std::move(black_list_);
            }
            if (auto it = settings.find(SettingsKeys::WHITELIST); it != settings.end()) {
                auto white_list_ = settings.at(SettingsKeys::WHITELIST).get<std::vector<std::string>>();
                white_list.reserve(white_list_.size());
                white_list = std::move(white_list_);
            }
            commands::user_validator::UserVerificator verificator(white_list, black_list);

            if (auto it = settings.find(SettingsKeys::ROLEFILTER_LEVEL); it != settings.end()) {
                int role_level = settings.at(SettingsKeys::ROLEFILTER_LEVEL).get<int>();
                verificator.SetRoleLevel(role_level);
            }
            if (auto it = settings.find(SettingsKeys::WHITELIST_ONLY); it != settings.end()) {
                bool is_whitelist_only = settings.at(SettingsKeys::WHITELIST_ONLY).get<bool>();
                verificator.SetWhiteListOnly(is_whitelist_only);
            }


            command_executor_->SetUserVerificator(std::move(verificator));
        }

        void LoadIRCClient(json& settings) {

        }
    };

} // namespace core