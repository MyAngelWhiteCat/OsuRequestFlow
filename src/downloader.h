#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "file_manager.h"
#include "random_user_agent.h"
#include "http_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include "logging.h"

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader(net::io_context& ioc, bool secured = true);

        ~Downloader(); // debug only;

        void Download(std::string_view file);
        void SetUserAgent(std::string_view user_agent);
        void SetUriPrefix(std::string_view uri_prefix);
        void SetResourse(std::string_view resourse);
        void SetDownloadsDirectory(std::string_view path);
        std::shared_ptr<http_domain::Client> SetupNonSecuredConnection();
        std::shared_ptr<http_domain::Client> SetupSecuredConnection();
        void SetMaxFileSize(size_t MiB);

        std::optional<std::string> GetResourse() const;
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
        std::optional<std::string> resourse_;
        std::optional<std::string> uri_prefix_;
        size_t max_file_size_MiB_ = 100;

        void OnDownload(std::string&& file_name, std::vector<char>&& body);

        void WriteOnDisk(std::string&& file_name, std::vector<char>&& bytes);

        std::string GetEndpoint(std::string_view file);

    };

}