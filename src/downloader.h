#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <unordered_map>

#include "file_manager.h"
#include "random_user_agent.h"
#include "http_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include <stdexcept>
#include "logging.h"

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using ResoursesAccess = std::unordered_map<std::string, std::vector<std::shared_ptr<http_domain::Client>>>;

    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx
            , std::vector<std::string> resourses
            , std::string_view user_agent
            , file_manager::FileManager file_manager
            , Strand& stream_strand);

        Downloader(net::io_context& ioc
            , std::vector<std::string> resourses
            , std::string_view user_agent
            , file_manager::FileManager file_manager
            , Strand& stream_strand);

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx
            , std::vector<std::string> resourses
            , std::shared_ptr<RandomUserAgent> user_agent
            , file_manager::FileManager file_manager
            , Strand& stream_strand);

        Downloader(net::io_context& ioc
            , std::vector<std::string> resourses
            , std::shared_ptr<RandomUserAgent> user_agent
            , file_manager::FileManager file_manager
            , Strand& stream_strand);

        ~Downloader() {
            LOG_INFO("Downloader destructed");
        }

        void Download(std::string_view uri);

    private:
        net::io_context& ioc_;
        Strand& file_write_strand_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        std::string user_agent_;
        std::shared_ptr<RandomUserAgent> user_agent_changer_{ nullptr };
        std::vector<std::string> resourses_;
        ResoursesAccess resourse_to_clients_;
        file_manager::FileManager file_manager_;
        bool secured_ = true;
        const size_t MAX_CONNECTIONS = 5;

        void OnDownload(std::string&& file_name, std::vector<char>&& body);

        void WriteOnDisk(std::string&& file_name, std::vector<char>&& bytes);

        void SetupConnection(std::shared_ptr<http_domain::Client> client, std::string_view port);

        bool ConnectToResourse(std::shared_ptr<http_domain::Client> client
            , std::string_view resourse, std::string_view port);

        void SetupNonSecuredConnection();

        void SetupSecuredConnection();

        void CheckResoursesForEmpty();

        void CleanUpInactiveConnections();

    };

}