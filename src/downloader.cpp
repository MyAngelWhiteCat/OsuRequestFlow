#include "downloader.h"
#include "file_manager.h"
#include "http_client.h"
#include "connection.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <unordered_map>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <stdexcept>
#include "logging.h"


namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using ResourcesAccess = std::unordered_map<std::string, std::vector<std::shared_ptr<http_domain::Client>>>;


    Downloader::Downloader(net::io_context& ioc, bool secured)
        : ioc_(ioc)
        , secured_(secured)
        , dl_strand(net::make_strand(ioc))
    {
    }

    void Downloader::SetupBaseServers(const std::vector<std::pair<std::string, std::string>>& servers) {
        for (const auto& server : servers) {
            base_servers_.emplace_back(server.first, server.second);
        }
    }

    void Downloader::MesureServersDownloadSpeed(std::string_view file) {
        for (auto& server : base_servers_) {
            MesureSpeed(server, file);
        }
    }

    void Downloader::MesureSpeed(Server& server, std::string_view to_file) {
        resource_ = server.host_;
        prefix_ = server.prefix_;
        try {
            auto client = GetReadyClient();
            client->SetSpeedMesureMode(true);
            client->Get(GetEndpoint(to_file), user_agent_, [self = this->shared_from_this(), client, &server]
            (http_domain::DLMetaData&& metadata) {
                    self->OnMesureSpeed(server, std::move(metadata));
                });
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }
    }

    void Downloader::Download(std::string_view file) {
        if (download_queue_.count(std::string(file))) {
            return;
        }
        auto now = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_dl_start_).count();
        if (dur < dl_timout_millisec_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(dur));
        }
        download_queue_.insert(std::string(file));
        LOG_INFO("Downdload "s.append(file));
        net::dispatch(dl_strand, [self = this->shared_from_this(), file = std::string(file)]() {
            try {
                auto client = self->GetReadyClient();
                client->Get(self->GetEndpoint(file), self->user_agent_, [self, client, file]
                (http_domain::DLMetaData&& metadata) {
                        self->OnDownload(file, std::move(metadata));
                    });
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
            }
            });
        last_dl_start_ = std::chrono::steady_clock::now();
    }

    void Downloader::SetUserAgent(std::string_view user_agent) {
        user_agent_ = std::string(user_agent);
    }

    void Downloader::SetUriPrefix(std::string_view uri_prefix) {
        prefix_ = std::string(uri_prefix);
    }

    void Downloader::SetResource(std::string_view resource) {
        resource_ = std::string(resource);
    }

    void Downloader::AddBaseServer(std::string_view host, std::string_view port) {
        base_servers_.emplace_back(host, port);
    }

    void Downloader::SetDownloadsDirectory(std::string_view path) {
        file_manager_ = std::make_shared<file_manager::FileManager>(std::filesystem::path(path));
    }

    void Downloader::OnDownload(std::string_view file, http_domain::DLMetaData&& metadata) {
        if (!metadata.success) {
            MesureServersDownloadSpeed(file);
            return;
        }
        download_queue_.erase(std::string(file));
        LOG_INFO("Successfuly download "s.append(std::to_string(static_cast<double>(metadata.file_size_) * http_domain::MiB)).append(" MB"));
        SaveAction(std::move(metadata.file_name_));
    }

    void Downloader::OnMesureSpeed(Server& server, http_domain::DLMetaData&& metadata) {
        if (!metadata.success) {
            server.status = ServerStatus::UNAVAILABLE;
            LOG_ERROR(server.host_ + " UNAVAILABLE");
            return;
        }
        LOG_ERROR(server.host_ + " EVAILABLE with speed "s.append(std::to_string(metadata.speed_mbs_).append("MB/s")));
        server.speed_mbs_ = metadata.speed_mbs_;
        std::sort(base_servers_.begin(), base_servers_.end());
        resource_ = base_servers_.back().host_;
        prefix_ = base_servers_.back().prefix_;
        if (!download_queue_.empty()) {
            std::string elem = *download_queue_.begin();
            download_queue_.erase(elem);
            Download(elem);
        }
    }

    void Downloader::SaveAction(std::string&& file_name) {
        net::post([self = this->shared_from_this(), file_name = std::move(file_name)]() mutable {
            self->file_manager_->AddAction(file_manager::ActionType::Write, std::move(file_name));
            });
    }

    std::shared_ptr<http_domain::Client> Downloader::SetupNonSecuredConnection() {
        if (!resource_) {
            throw std::runtime_error("Resource doesnt setted");
        }
        auto client = std::make_shared<http_domain::Client>(ioc_);
        client->Connect(*resource_, http_domain::Port::NON_SECURED);
        return client;
    }

    std::shared_ptr<http_domain::Client> Downloader::SetupSecuredConnection() {
        if (!resource_) {
            throw std::runtime_error("Resource doesn't setted");
        }
        auto client = std::make_shared<http_domain::Client>(ioc_, connection::GetSSLContext());
        client->SetMaxFileSize(max_file_size_MiB_);
        client->Connect(*resource_, http_domain::Port::SECURED);
        return client;
    }

    void Downloader::SetMaxFileSize(size_t MiB) {
        max_file_size_MiB_ = MiB;
    }

    std::optional<std::string> Downloader::GetResource() const {
        return resource_;
    }

    std::optional<std::string> Downloader::GetPrefix() const {
        return prefix_;
    }

    size_t Downloader::GetMaxFileSize() const {
        return max_file_size_MiB_;
    }

    std::optional<std::filesystem::path> Downloader::GetDownloadsDirectory() const {
        if (file_manager_) {
            return file_manager_->GetRootDirectory();
        }
        return std::nullopt;
    }

    std::string Downloader::GetUserAgent() const {
        return user_agent_;
    }

    std::string Downloader::GetEndpoint(std::string_view file) {
        if (!prefix_) {
            throw std::runtime_error("prefix doesn't setted");
        }
        return *prefix_ + std::string(file);
    }

    std::shared_ptr<http_domain::Client> Downloader::GetReadyClient() {
        std::shared_ptr<http_domain::Client> client = nullptr;
        if (secured_) {
            client = SetupSecuredConnection();
        }
        else {
            client = SetupNonSecuredConnection();
        }
        client->SetMaxFileSize(max_file_size_MiB_);
        if (auto dir = GetDownloadsDirectory()) {
            client->SetRootDirectory(*dir);
        }
        else {
            throw std::runtime_error("Download directory not setted");
        }
        return client;
    }

} // namespace downloader