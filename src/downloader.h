#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>

#include "osu_file_manager.h"
#include "http_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include "logging.h"

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    struct AccessTestResult {
        static constexpr std::string_view AVAILABLE = "Available"sv;
        static constexpr std::string_view UNAVAILABLE = "Unavailable"sv;
        static constexpr std::string_view PROCESSING = "Processing"sv;
    };

    enum class ServerStatus {
        AVAILABLE,
        UNAVAILABLE,
        UNKNOWN
    };

    struct Server {
        Server(std::string_view host, std::string_view prefix)
            : host_(std::string(host))
            , prefix_(std::string(prefix))
        {
        }

        std::string host_;
        std::string prefix_;
        std::optional<double> speed_mbs_;
        ServerStatus status = ServerStatus::UNKNOWN;

        bool operator<(const Server& other) const {
            if (speed_mbs_) {
                if (other.speed_mbs_) {
                    return *speed_mbs_ < *other.speed_mbs_;
                }
                else {
                    return false;
                }
            }
            else {
                if (other.speed_mbs_) {
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        bool operator==(const Server& other) const {
            return host_ == other.host_ && prefix_ == other.prefix_;
        }
    };


    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader(net::io_context& ioc, bool secured = true);
        ~Downloader() { LOG_DEBUG("Downloader destructed"); }

        void SetupBaseServers(const std::vector<std::pair<std::string, std::string>>& servers);
        void SetUserAgent(std::string_view user_agent);
        void SetResourceAndPrefix(std::string_view resource, std::string_view uri_prefix);
        void AddBaseServer(std::string_view host, std::string_view port);
        void SetDownloadsDirectory(std::string_view path);
        std::shared_ptr<http_domain::Client> SetupNonSecuredConnection();
        std::shared_ptr<http_domain::Client> SetupSecuredConnection();
        void SetMaxFileSize(size_t MiB);

        void MesureServersDownloadSpeed(std::string_view file);
        void MesureSpeed(Server& server, std::string_view to_file);
        void Download(std::string_view file);
        bool IsNeedToMesureSpeed() const;
        std::string GetAccessTestResult();

        std::optional<std::string> GetResource() const;
        std::optional<std::string> GetPrefix() const;
        size_t GetMaxFileSize() const;
        std::optional<std::filesystem::path> GetDownloadsDirectory() const;
        std::string GetUserAgent() const;

    private:
        net::io_context& ioc_;
        Strand dl_strand_;
        Strand dl_status_strand_;
        bool secured_ = true;
        bool is_speed_mesured_ = false;
        bool is_any_available_ = false;

        std::string user_agent_ = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

        std::shared_ptr<osu_file_manager::OsuFileManager> osu_file_manager_{ nullptr };
        std::optional<std::string> resource_;
        std::optional<std::string> prefix_;

        std::vector<Server> base_servers_;
        std::unordered_map<size_t, std::optional<http_domain::Client>> server_to_client_;
        std::unordered_set<std::string> download_queue_;

        size_t max_file_size_MiB_ = 100;
        const int dl_timout_millisec_ = 1000;
        std::chrono::steady_clock::time_point last_dl_start_;
        bool need_to_mesure_speed_ = true;

        void OnDownload(std::string_view file, http_domain::DLMetaData&& metadata);
        void OnMesureSpeed(Server& server, http_domain::DLMetaData&& metadata);
        void SaveAction(std::string&& file_name);
        std::string GetEndpoint(std::string_view file);
        std::shared_ptr<http_domain::Client> GetReadyClient();

        bool IsSpeedMesured();
        bool IsAnyResourseAvailable();
    };

}