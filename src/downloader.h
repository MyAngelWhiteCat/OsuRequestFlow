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

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using ResoursesAccess = std::unordered_map<std::string, std::shared_ptr<http_domain::Client>>;

    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx
            , std::vector<std::string> resourses
            , std::string_view user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , ctx_(ctx)
            , resourses_(resourses)
            , user_agent_(user_agent)
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupFirstConnection();
        }

        Downloader(net::io_context& ioc
            , std::vector<std::string> resourses
            , std::string_view user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , resourses_(resourses)
            , user_agent_(user_agent)
            , file_manager_(file_manager)

        {
            CheckResoursesForEmpty();
            SetupFirstConnection();
        }

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx
            , std::vector<std::string> resourses
            , RandomUserAgent user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , ctx_(ctx)
            , resourses_(resourses)
            , user_agent_changer_(std::make_unique<RandomUserAgent>(user_agent))
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupFirstConnection();
        }

        Downloader(net::io_context& ioc
            , std::vector<std::string> resourses
            , RandomUserAgent user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , resourses_(resourses)
            , user_agent_changer_(std::make_unique<RandomUserAgent>(user_agent))
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupFirstConnection();
        }

        void Download(std::string_view uri) {
            if (user_agent_changer_) {
                user_agent_ = user_agent_changer_->GetUserAgent();
            }
            for (const auto& [resourse, client] : resourse_to_client_) {
                if (client->IsConnected()) {
                    client->Get(uri, user_agent_, [self = shared_from_this()]
                    (std::unordered_map<std::string, std::string>&& headers_to_value, std::string&& body) {
                            self->OnDownload(std::move(headers_to_value), std::move(body));
                        });
                }
            }
        }

    private:
        net::io_context& ioc_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        std::string user_agent_;
        std::unique_ptr<RandomUserAgent> user_agent_changer_{ nullptr };
        std::vector<std::string> resourses_;
        ResoursesAccess resourse_to_client_;
        file_manager::FileManager file_manager_;
        bool secured_ = true;

        void OnDownload(std::unordered_map<std::string, std::string> headers_to_value, std::string body) {
            // I am not sure about map name.. i need to find out does it have any sense...
        }

        void WriteOnDisk(std::string&& bytes, std::string&& file_name) {
            net::post(ioc_, [self = this->shared_from_this()
                , bytes = std::move(bytes), file_name = std::move(file_name)]() mutable {
                self->file_manager_.WriteInRoot(std::move(bytes), std::move(file_name));
                });
        }

        void SetupFirstConnection() {
            std::shared_ptr<http_domain::Client> client;
            if (ctx_) {
                client = SetupSecuredConnection();
            }
            else {
                client = SetupNonSecuredConnection();
            }

        }

        std::shared_ptr<http_domain::Client> SetupConnection(std::shared_ptr<http_domain::Client> client
            , std::string_view port) {
            for (const auto& resourse : resourses_) {
                client->Connect(resourse, port);
                if (client->IsConnected()) {
                    return client;
                }
            }
            throw std::runtime_error("All resourses unavailable");
        }

        std::shared_ptr<http_domain::Client> SetupNonSecuredConnection() {
            auto client = std::make_shared<http_domain::Client>(ioc_);
            return SetupConnection(client, http_domain::Port::NON_SECURED);

        }

        std::shared_ptr<http_domain::Client> SetupSecuredConnection() {
            auto client = std::make_shared<http_domain::Client>(ioc_);
            return SetupConnection(client, http_domain::Port::SECURED);
        }

        void CheckResoursesForEmpty() {
            if (resourses_.empty()) {
                throw std::runtime_error("Empty resourses");
            }
        }

    };

}