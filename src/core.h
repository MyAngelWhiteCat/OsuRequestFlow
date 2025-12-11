#pragma once

#include "irc_client.h"
#include "auth_data.h"
#include "command_executor.h"
#include "command_parser.h"
#include "nlohmann/json.hpp"

#include <memory>
#include <string_view>


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
        static constexpr std::string_view USER_VERIFICATOR = "UserVerificator"sv;
        static constexpr std::string_view WHITELIST = "Whitelist"sv;
        static constexpr std::string_view BLACKLIST = "Blacklist"sv;
        static constexpr std::string_view ROLEFILTER_LEVEL = "RolefilterLevel"sv;
        static constexpr std::string_view WHITELIST_ONLY = "WhiteListOnly"sv;
        static constexpr std::string_view IRC_CLIENT = "IRCClient"sv;
        static constexpr std::string_view NAME = "Name"sv;
        static constexpr std::string_view COLOR = "Color"sv;
        static constexpr std::string_view BADGE = "Badge"sv;
        static constexpr std::string_view MESSAGE = "Message"sv;
        static constexpr std::string_view JOINED_CHANNELS = "JoinedChannels"sv;
        static constexpr std::string_view RECONNECT_TIMEOUT = "ReconnectTimeout"sv;
        static constexpr std::string_view DOWNLOADER = "Downloader"sv;
        static constexpr std::string_view MAX_FILESIZE = "MaxFileSize"sv;
        static constexpr std::string_view USER_AGENT = "UserAgent"sv;
        static constexpr std::string_view DOWNLOADS_DIR = "DownloadsDirectory"sv;
        static constexpr std::string_view RESOURCE = "Resource"sv;
        static constexpr std::string_view PREFIX = "Prefix"sv;
    };

    class Core {
    public:
        Core(net::io_context& ioc)
            : ioc_(ioc)
        {

        }

        void SetupConnection(bool secured);
        void SetupDownloader(bool secured, std::string_view resource
            , std::string_view uri_prefix, std::string_view downloads_directory);
        void SetupDownloader(bool secured);
        void SetupChatBot();
        void SetAuthData(std::string_view nick, std::string_view tocken);
        void SetupIRCClient(bool secured);
        void Start();

        // settings

        void SaveSettings();
        void LoadSettings();

        // user_validator

        std::unordered_set<std::string>* GetWhiteList();
        std::unordered_set<std::string>* GetBlackList();
        bool IsUserInWhiteList(std::string_view username);
        bool IsUserInBlackList(std::string_view username);
        void AddUserInWhiteList(std::string_view user);
        void AddUserInBlackList(std::string_view user);
        void RemoveUserFromWhiteList(std::string_view user);
        void RemoveUserFromBlackList(std::string_view user);
        void SetRoleLevelFilter(int level);
        void SetWhiteListOnly(bool on);

        int GetRoleLevelFilter();
        bool GetWhiteListOnly();

        // downloader

        void SetDownloadResourceAndPrefix(std::string_view resource, std::string_view prefix);
        void SetDownloadsDirectory(std::string_view path);
        void SetMaxFileSize(size_t MiB);

        void MesureDownloadSpeed(std::string_view to_resourse = "1886002");

        std::optional<std::string_view> GetDownloadResource();
        std::optional<std::string_view> GetDownloadPrefix();
        std::optional<std::filesystem::path> GetDownloadsDirectory();
        size_t GetMaxFileSize();

        // irc client

        void Join(std::string_view channel);
        void Part(std::string_view channel);
        void SetReconnectTimeout(int seconds);
        
        std::vector<std::string_view> GetJoinedChannels();

        // Chat Widget

        void ShowChat(bool toggle);

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

        void CheckReadyness();
        json GetUserVerificatorSettings();
        void LoadUserVerificatorSettings(json& settings);

        json GetIRCClientSettings();
        void LoadIRCClientSettings(json& settings);

        json GetDownloaderSettings();
        void LoadDownloaderSettings(json& settings);

        void SetBaseDownloaderResources();

    };

} // namespace core