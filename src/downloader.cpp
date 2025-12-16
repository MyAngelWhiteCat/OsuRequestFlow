#include "downloader.h"
#include "osu_file_manager.h"
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
        , dl_strand_(net::make_strand(ioc))
        , dl_status_strand_(net::make_strand(ioc))
        , write_dl_status_strand_(net::make_strand(ioc))
    {
        LOG_DEBUG("Downloader constructed");
    }

    void Downloader::SetupBaseServers(const std::vector<std::pair<std::string, std::string>>& servers) {
        for (const auto& server : servers) {
            LOG_DEBUG("Set up base server: "s.append(server.first).append(server.second));
            base_servers_.emplace_back(server.first, server.second);
        }
    }

    void Downloader::MesureServersDownloadSpeed(std::string_view file) {
        for (auto& server : base_servers_) {
            MesureSpeed(server, file);
        }
        need_to_mesure_speed_ = false;
    }

    void Downloader::MesureSpeed(Server& server, std::string_view to_file) {
        LOG_CRITICAL(std::to_string(base_servers_.size()));
        net::dispatch(dl_strand_, [self = shared_from_this(), &server, to_file = std::string(to_file)]() {
            if (!self->need_to_mesure_speed_) { return; }
            self->resource_ = server.host_;
            self->prefix_ = server.prefix_;
            for (auto& server : self->base_servers_) {
                server.status = ServerStatus::UNKNOWN;
            }
            });

        LOG_INFO("Mesuring dl speed to "s.append(server.host_).append(server.prefix_));
        try {
            auto client = GetReadyClient();
            client->SetSpeedMesureMode(true);
            client->Get(GetEndpoint(to_file), user_agent_, [self = this->shared_from_this(), client, &server]
            (http_domain::DLMetaData&& metadata) {
                    self->OnMesureSpeed(server, std::move(metadata));
                });
        }
        catch (const std::exception& e) {
            need_to_mesure_speed_ = true;
            LOG_CRITICAL(e.what());
        }
    }

    void Downloader::Download(std::string_view file) {
        //if (osu_file_manager_->IsAlreadyInstalled(file)) {
        //    LOG_INFO("Map already exist");
        //    return;
        //}
        if (download_queue_.count(std::string(file))) {
            LOG_INFO("File allready in dl queue");
            return;
        }
        auto now = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_dl_start_).count();
        if (dur < dl_timout_millisec_) {
            LOG_INFO("DL Timeout");
            std::this_thread::sleep_for(std::chrono::milliseconds(dur));
        }

        download_queue_.insert(std::string(file));
        LOG_INFO("Downdload "s.append(file));
        net::dispatch(dl_strand_, [self = this->shared_from_this(), file = std::string(file)]() {
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

    bool Downloader::IsNeedToMesureSpeed() const {
        return need_to_mesure_speed_;
    }

    bool Downloader::IsSpeedMesured() {
        bool mesured = true;
        for (const auto& server : base_servers_) {
            if (server.status == ServerStatus::UNKNOWN) {
                mesured = false;
            }
        }
        return mesured;
    }

    bool Downloader::IsAnyResourseAvailable() {
        bool available = false;
        for (auto& server : base_servers_) {
            if (server.status == ServerStatus::AVAILABLE) {
                available = true;
            }
        }
        return available;
    }

    std::string Downloader::GetAccessTestResult() {
        net::dispatch(dl_status_strand_, [self = this->shared_from_this()]() mutable {
            self->is_speed_mesured_ = self->IsSpeedMesured();
            self->is_any_available_ = self->IsAnyResourseAvailable(); 
            });

        if (is_speed_mesured_) {
            if (is_any_available_) {
                return std::string(AccessTestResult::AVAILABLE);
            }
            return std::string(AccessTestResult::UNAVAILABLE);
        }
        if (is_any_available_) {
            return std::string(AccessTestResult::AVAILABLE);
        }
        return std::string(AccessTestResult::PROCESSING);
    }

    void Downloader::SetUserAgent(std::string_view user_agent) {
        user_agent_ = std::string(user_agent);
    }

    void Downloader::SetResourceAndPrefix(std::string_view resource, std::string_view uri_prefix) {
        prefix_ = std::string(uri_prefix);
        resource_ = std::string(resource);
        Server server(resource, uri_prefix);
        if (std::find(base_servers_.begin(), base_servers_.end(), server) == base_servers_.end()) {
            base_servers_.push_back(std::move(server));
        }
    }

    void Downloader::AddBaseServer(std::string_view host, std::string_view prefix) {
        Server server(host, prefix);
        if (std::find(base_servers_.begin(), base_servers_.end(), server) == base_servers_.end()) {
            base_servers_.push_back(std::move(server));
        }
    }

    void Downloader::SetDownloadsDirectory(std::string_view path) {
        osu_file_manager_ = std::make_shared<osu_file_manager::OsuFileManager>(std::filesystem::path(path));
    }

    void Downloader::OnDownload(std::string_view file, http_domain::DLMetaData&& metadata) {
        if (!metadata.success) {
            osu_file_manager_->RemoveFileFromRoot(metadata.file_name_);
            need_to_mesure_speed_ = true;
            MesureServersDownloadSpeed(file);
            return;
        }
        download_queue_.erase(std::string(file));
        LOG_INFO("Successfuly download "s.append(std::to_string(static_cast<double>(metadata.file_size_) / http_domain::MiB)).append(" MB"));
    }

    void Downloader::OnMesureSpeed(Server& server, http_domain::DLMetaData&& metadata_) {
        http_domain::DLMetaData metadata(std::move(metadata_));
        net::post(write_dl_status_strand_, [metadata] {
            std::ofstream log("LogAccessTestResult.txt", std::ios::app);
            metadata.Print(log);
            });

        need_to_mesure_speed_ = !metadata.success;
        if (!metadata.success) {
            server.status = ServerStatus::UNAVAILABLE;
            LOG_ERROR(server.host_ + " UNAVAILABLE");
            return;
        }
        server.status = ServerStatus::AVAILABLE;
        LOG_INFO(server.host_ + " EVAILABLE with speed "s.append(std::to_string(metadata.speed_mbs_).append("MB/s")));
        server.speed_mbs_ = metadata.speed_mbs_;
        std::sort(base_servers_.begin(), base_servers_.end());
        std::string servers;
        for (const auto& server : base_servers_) {
            servers += server.host_ + " speed=";
            if (server.speed_mbs_) {
                servers += std::to_string(*server.speed_mbs_) + "MB/s ";
            }
            else {
                if (server.status == ServerStatus::UNAVAILABLE) {
                    servers += "UNAVAILABLE ";
                }
                else {
                    servers += "Not mesured ";
                }
            }
        }

        LOG_INFO("Servers after speed based sort: "s.append(servers));
        resource_ = base_servers_.back().host_;
        prefix_ = base_servers_.back().prefix_;
        if (!download_queue_.empty()) {
            std::string dl_queue;
            for (const auto& elem : download_queue_) {
                dl_queue += elem + " ";
            }
            LOG_INFO(std::to_string(download_queue_.size()).append(" elements in download queue: ").append(dl_queue));
            std::string elem = *download_queue_.begin();
            download_queue_.erase(elem);
            Download(elem);
        }


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
        if (osu_file_manager_) {
            return osu_file_manager_->GetRootDirectory();
        }
        return std::nullopt;
    }

    std::string Downloader::GetUserAgent() const {
        return user_agent_;
    }

    void Downloader::RemoveDublicatesInRootDirectory() {
        if (!osu_file_manager_) {
            throw std::logic_error("need to setup file manager");
        }
        osu_file_manager_->RemoveDuplicates();
    }

    std::string Downloader::GetEndpoint(std::string_view file) {
        if (!prefix_) {
            throw std::runtime_error("prefix doesn't setted");
        }
        LOG_INFO("DL LINK "s.append(*prefix_ + std::string(file)));
        return *prefix_ + std::string(file);
    }

    std::shared_ptr<http_domain::Client> Downloader::GetReadyClient() {
        std::shared_ptr<http_domain::Client> client = nullptr;
        if (secured_) {
            LOG_INFO("Create secured client");
            client = SetupSecuredConnection();
        }
        else {
            LOG_INFO("Create non secured client");
            client = SetupNonSecuredConnection();
        }
        client->SetMaxFileSize(max_file_size_MiB_);
        if (auto dir = GetDownloadsDirectory()) {
            LOG_INFO("Setting dl dir: "s.append(dir->string()));
            client->SetRootDirectory(*dir);
        }
        else {
            if (!need_to_mesure_speed_) {
                LOG_ERROR("Creating dl client withoud dl directory");
                throw std::runtime_error("Download directory not setted");
            }
        }
        return client;
    }

} // namespace downloader