#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

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

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx);

        Downloader(net::io_context& ioc);

        ~Downloader(); // debug only;

        void Download(std::string_view uri);
        void SetUserAgent(std::string_view user_agent);
        void SetUriPrefix(std::string_view uri_prefix);
        void SetResourse(std::string_view resourse);
        void SetDownloadsFolder(std::string_view path);
        void SetupNonSecuredConnection();
        void SetupSecuredConnection();
        void SetMaxFileSize(int MiB);

    private:
        net::io_context& ioc_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        std::string user_agent_ = "OsuRequestFlow v0.1";
        std::shared_ptr<http_domain::Client> client_{ nullptr };
        std::shared_ptr<file_manager::FileManager> file_manager_{ nullptr };
        std::string resourse_;
        std::string uri_prefix_ = "/d/";

        void OnDownload(std::string&& file_name, std::vector<char>&& body);

        void WriteOnDisk(std::string&& file_name, std::vector<char>&& bytes);

        std::string GetEndpoint(std::string_view file);

    };

}