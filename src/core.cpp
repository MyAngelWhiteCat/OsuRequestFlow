#include "core.h"


namespace core {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using json = nlohmann::json;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;


    void core::Core::SetupConnection(bool secured) {
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

    void Core::SetupDownloader(bool secured, std::string_view resource
        , std::string_view uri_prefix, std::string_view downloads_directory) {
        downloader_ = std::make_shared<downloader::Downloader>(ioc_, secured);
        downloader_->SetResourceAndPrefix(resource, uri_prefix);
        downloader_->SetDownloadsDirectory(downloads_directory);
        SetBaseDownloaderResources();
        SetupChatBot();
    }

    void Core::SetupDownloader(bool secured) {
        downloader_ = std::make_shared<downloader::Downloader>(ioc_, secured);
        SetBaseDownloaderResources();
        SetupChatBot();
    }

    void Core::SetupChatBot() {
        if (!downloader_) {
            throw std::logic_error("Setup downloader before setup chat bot");
        }

        command_executor_ = std::make_shared<commands::CommandExecutor>(downloader_);
        command_parser_ = std::make_shared<commands::CommandParser>(*command_executor_);
        chat_bot_ = std::make_shared<chat_bot::ChatBot>(command_executor_, command_parser_);
    }

    void Core::SetAuthData(std::string_view nick, std::string_view tocken) {
        auth_data_.SetNick(nick);
        auth_data_.SetToken(tocken);
    }

    void Core::SetupIRCClient(bool secured) {
        client_ = std::make_shared<irc::Client<chat_bot::ChatBot>>(ioc_, chat_bot_, secured);
    }

    void Core::Start() {
        CheckReadyness();
        client_->Connect();
        client_->Authorize(auth_data_);
        client_->CapRequest();
        client_->Read();
    }

    // settings

    void Core::SaveSettings() {
        json settings;
        settings[SettingsKeys::USER_VERIFICATOR] = GetUserVerificatorSettings();
        settings[SettingsKeys::IRC_CLIENT] = GetIRCClientSettings();
        settings[SettingsKeys::DOWNLOADER] = GetDownloaderSettings();
        std::ofstream out(std::string(SettingsKeys::FILENAME.data(), SettingsKeys::FILENAME.size()));
        out << settings.dump(4);
    }

    void Core::LoadSettings() {
        json settings;
        std::ifstream in(std::string(SettingsKeys::FILENAME.data(), SettingsKeys::FILENAME.size()));
        in >> settings;
        std::cout << settings << std::endl;
        if (auto it = settings.find(SettingsKeys::USER_VERIFICATOR); it != settings.end()) {
            LoadUserVerificatorSettings(*it);
        }
        if (auto it = settings.find(SettingsKeys::IRC_CLIENT); it != settings.end()) {
            LoadIRCClientSettings(*it);
        }
        if (auto it = settings.find(SettingsKeys::DOWNLOADER); it != settings.end()) {
            LoadDownloaderSettings(*it);
        }
    }

    std::unordered_set<std::string>* Core::GetWhiteList() {
        return command_executor_->GetUserVerificator()->GetWhiteList();
    }

    std::unordered_set<std::string>* Core::GetBlackList() {
        return command_executor_->GetUserVerificator()->GetBlackList();
    }

    bool Core::IsUserInWhiteList(std::string_view username) {
        return command_executor_->GetUserVerificator()->GetWhiteList()->count(std::string(username));
    }

    bool Core::IsUserInBlackList(std::string_view username) {
        return command_executor_->GetUserVerificator()->GetBlackList()->count(std::string(username));
    }

    void Core::AddUserInWhiteList(std::string_view user) {
        command_executor_->GetUserVerificator()->AddUserInWhiteList(user);
    }

    void Core::AddUserInBlackList(std::string_view user) {
        command_executor_->GetUserVerificator()->AddUserInBlackList(user);
    }

    void Core::RemoveUserFromWhiteList(std::string_view user) {
        command_executor_->GetUserVerificator()->RemoveUserFromWhiteList(user);
    }

    void Core::RemoveUserFromBlackList(std::string_view user) {
        command_executor_->GetUserVerificator()->RemoveUserFromBlackList(user);
    }

    void Core::SetRoleLevelFilter(int level) {
        command_executor_->GetUserVerificator()->SetRoleLevel(level);
    }

    void Core::SetWhiteListOnly(bool on) {
        command_executor_->GetUserVerificator()->SetWhiteListOnly(on);
    }

    int Core::GetRoleLevelFilter() {
        return command_executor_->GetUserVerificator()->GetRoleLevel();
    }

    bool Core::GetWhiteListOnly() {
        return command_executor_->GetUserVerificator()->GetWhiteListOnly();
    }

    void Core::ShowChat(bool toggle) {
        // init websocket server 
    }

    // downloader

    void Core::SetDownloadResourceAndPrefix(std::string_view resource, std::string_view prefix) {
        downloader_->SetResourceAndPrefix(resource, prefix);
    }

    void Core::SetDownloadsDirectory(std::string_view path) {
        downloader_->SetDownloadsDirectory(path);
    }

    void Core::PickDownloadsDirectory() {
        std::string path = SelectFolderDialog();
        if (!path.empty()) {
            downloader_->SetDownloadsDirectory(path);
    }
    }

    void Core::SetMaxFileSize(size_t MiB) {
        downloader_->SetMaxFileSize(MiB);
    }

    bool Core::IsNeedToMesureSpeed() {
        return downloader_->IsNeedToMesureSpeed();
    }

    std::string Core::GetAccessTestResult() {
        return downloader_->GetAccessTestResult();
    }

    void Core::MesureDownloadSpeed(std::string_view to_resourse) {
        downloader_->MesureServersDownloadSpeed(to_resourse);
    }

    std::optional<std::string_view> Core::GetDownloadResource() {
        return downloader_->GetResource();
    }

    std::optional<std::string_view> Core::GetDownloadPrefix() {
        return downloader_->GetPrefix();
    }

    std::optional<std::filesystem::path> Core::GetDownloadsDirectory() {
        return downloader_->GetDownloadsDirectory();
    }

    size_t Core::GetMaxFileSize() {
        return downloader_->GetMaxFileSize();
    }

    // irc client

    void Core::Join(std::string_view channel) {
        client_->Join(channel);
    }

    void Core::Part(std::string_view channel) {
        client_->Part(channel);
    }

    void Core::SetReconnectTimeout(int seconds) {
        client_->SetReconnectTimeout(seconds);
    }

    std::vector<std::string_view> Core::GetJoinedChannels() {
        auto jc = client_->GetJoinedChannels();
        std::vector<std::string_view> joined_channels;
        joined_channels.reserve(jc.size());

        for (const auto& channel : jc) {
            joined_channels.push_back(channel);
        }

        return joined_channels;
    }

    // private

    void Core::CheckReadyness() {
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

    json Core::GetUserVerificatorSettings() {
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

    void Core::LoadUserVerificatorSettings(json& settings) {
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

    json Core::GetIRCClientSettings() {
        json settings;
        if (!client_) {
            throw std::logic_error("Cant save uninitialized client's settings");
        }
        std::vector<std::string> joined_channels(client_->GetJoinedChannels().begin()
                                               , client_->GetJoinedChannels().end());
        settings[SettingsKeys::JOINED_CHANNELS] = std::move(joined_channels);
        settings[SettingsKeys::RECONNECT_TIMEOUT] = client_->GetReconnectTimeout();
        return settings;
    }

    void Core::LoadIRCClientSettings(json& settings) {
        if (!client_) {
            throw std::logic_error("Cant load uninitialized client's settings");
        }
        if (auto it = settings.find(SettingsKeys::JOINED_CHANNELS); it != settings.end()) {
            if (it->is_array()) {
                client_->Join(it->get<std::vector<std::string_view>>());
            }
            else {
                throw std::runtime_error("Invalid settings JSON");
            }
        }
        if (auto it = settings.find(SettingsKeys::RECONNECT_TIMEOUT); it != settings.end()) {
            if (it->is_number_integer()) {
                client_->SetReconnectTimeout(it->get<int>());
            }
            else {
                throw std::runtime_error("Invalid settings JSON");
            }
        }
    }

    json Core::GetDownloaderSettings() {
        json settings;
        if (!downloader_) {
            throw std::logic_error("Cant save uninitialized downloader's settings");
        }
        settings[SettingsKeys::MAX_FILESIZE] = downloader_->GetMaxFileSize();
        settings[SettingsKeys::USER_AGENT] = downloader_->GetUserAgent();
        if (auto dir = downloader_->GetDownloadsDirectory()) {
            settings[SettingsKeys::DOWNLOADS_DIR] = *dir;
        }
        if (auto res = downloader_->GetResource()) {
            settings[SettingsKeys::RESOURCE] = *res;
        }
        if (auto prefix = downloader_->GetPrefix()) {
            settings[SettingsKeys::PREFIX] = *prefix;
        }
        return settings;
    }

    void Core::LoadDownloaderSettings(json& settings) {
        if (!downloader_) {
            throw std::logic_error("Cant laod uninitialized downloader's settings");
        }
        if (auto it = settings.find(SettingsKeys::MAX_FILESIZE); it != settings.end()) {
            downloader_->SetMaxFileSize(it->get<size_t>());
        }
        if (auto it = settings.find(SettingsKeys::USER_AGENT); it != settings.end()) {
            downloader_->SetUserAgent(it->get<std::string>());
        }
        if (auto it = settings.find(SettingsKeys::DOWNLOADS_DIR); it != settings.end()) {
            downloader_->SetDownloadsDirectory(it->get<std::string_view>());
        }
        if (auto it = settings.find(SettingsKeys::RESOURCE); it != settings.end()) {
            downloader_->SetResource(it->get<std::string>());
        }
        if (auto it = settings.find(SettingsKeys::PREFIX); it != settings.end()) {
            downloader_->SetUriPrefix(it->get<std::string>());
        }
    }

    void Core::SetBaseDownloaderResources() {
        std::vector<std::pair<std::string, std::string>> base_resources{
            {"osu.direct", "/api/d/"},
            {"catboy.best", "/d/"},
            {"api.nerinyan.moe", "/d/"}
        };
        downloader_->SetupBaseServers(base_resources);
    }

    std::string Core::SelectFolderDialog() {
        // AI on
        std::string result;

        CoInitialize(NULL);

        IFileDialog* pfd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pfd)))) {
            DWORD dwOptions;
            pfd->GetOptions(&dwOptions);
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);

            if (SUCCEEDED(pfd->Show(NULL))) {
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi))) {
                    PWSTR pszPath;
                    if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                        char buffer[MAX_PATH];
                        wcstombs(buffer, pszPath, MAX_PATH);
                        result = buffer;
                        CoTaskMemFree(pszPath);
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }

        CoUninitialize();
        return result;
        // AI off
    }


}