#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "file_manager.h"
#include "http_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    struct Server {
        std::string host_;
        std::string prefix_;
        double speed_mbs_ = 0;

        bool operator<(const Server& other) {
            return speed_mbs_ < other.speed_mbs_;
        }

        bool operator==(const Server& other) {
            return host_ == other.host_ && prefix_ == other.prefix_;
        }
    };



    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader(net::io_context& ioc, bool secured = true);

        void Download(std::string_view file);
        void SetUserAgent(std::string_view user_agent);
        void SetUriPrefix(std::string_view uri_prefix);
        void SetResource(std::string_view resource);
        void SetDownloadsDirectory(std::string_view path);
        std::shared_ptr<http_domain::Client> SetupNonSecuredConnection();
        std::shared_ptr<http_domain::Client> SetupSecuredConnection();
        void SetMaxFileSize(size_t MiB);

        std::optional<std::string> GetResource() const;
        std::optional<std::string> GetPrefix() const;
        size_t GetMaxFileSize() const;
        std::optional<std::filesystem::path> GetDownloadsDirectory() const;
        std::string GetUserAgent() const;

    private:
        net::io_context& ioc_;
        Strand dl_strand;
        bool secured_ = true;
        std::string user_agent_ = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

        std::shared_ptr<file_manager::FileManager> file_manager_{ nullptr };
        std::optional<std::string> resource_;
        std::optional<std::string> uri_prefix_;

        size_t max_file_size_MiB_ = 100;
        const int dl_timout_millisec_ = 1000;
        std::chrono::steady_clock::time_point last_dl_start_;

        void OnDownload(std::string&& file_name, size_t bytes_downloaded);
        void SaveAction(std::string&& file_name);
        std::string GetEndpoint(std::string_view file);
        std::shared_ptr<http_domain::Client> GetReadyClient();

    };

}