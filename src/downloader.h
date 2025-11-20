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
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , ctx_(ctx)
            , resourses_(resourses)
            , user_agent_(user_agent)
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupSecuredConnection();
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
            SetupNonSecuredConnection();
        }

        Downloader(net::io_context& ioc
            , std::shared_ptr<ssl::context> ctx
            , std::vector<std::string> resourses
            , std::shared_ptr<RandomUserAgent> user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , ctx_(ctx)
            , resourses_(resourses)
            , user_agent_changer_(user_agent)
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupSecuredConnection();
        }

        Downloader(net::io_context& ioc
            , std::vector<std::string> resourses
            , std::shared_ptr<RandomUserAgent> user_agent
            , file_manager::FileManager file_manager)
            : ioc_(ioc)
            , resourses_(resourses)
            , user_agent_changer_(user_agent)
            , file_manager_(file_manager)
        {
            CheckResoursesForEmpty();
            SetupNonSecuredConnection();
        }

        ~Downloader() {
            LOG_INFO("Downloader destructed");
        }

        void Download(std::string_view uri) {
            LOG_INFO("DOWNLOAD");
            if (user_agent_changer_) {
                user_agent_ = user_agent_changer_->GetUserAgent();
                LOG_INFO("Set User Agent");

            }
            for (auto& [resourse, clients] : resourse_to_clients_) {
                LOG_INFO("Check resourse");
                for (auto& client : clients) {
                    if (!client->IsConnected()) {
                        if (ctx_) {
                            SetupSecuredConnection();
                        }
                    }
                    if (!client->IsBusy()){
                        client->Get(uri, user_agent_, [self = this->shared_from_this()]
                        (std::unordered_map<std::string, std::string>&& headers_to_value, std::vector<char>&& body) {
                                self->OnDownload(std::move(headers_to_value), std::move(body));
                            });
                        return;
                    }
                    else {
                        
                    }
                }
            }
        }
    

    private:
        net::io_context& ioc_;
        std::shared_ptr<ssl::context> ctx_{ nullptr };
        std::string user_agent_;
        std::shared_ptr<RandomUserAgent> user_agent_changer_{ nullptr };
        std::vector<std::string> resourses_;
        ResoursesAccess resourse_to_clients_;
        file_manager::FileManager file_manager_;
        std::mutex m_;
        bool secured_ = true;

        void OnDownload(std::unordered_map<std::string, std::string> headers_to_value, std::vector<char> body) {
            // I am not sure about map name.. i need to find out does it have any sense...
            std::string bytes = std::to_string(body.size());
            LOG_INFO("Successfuly download "s.append(bytes).append(" bytes"));
            LOG_INFO("Headers:");
            for (const auto& [header, value] : headers_to_value) {
                LOG_INFO(header + ": " + value);
            }
            WriteOnDisk(std::move(body), std::move(std::to_string(body.size()).append(".txt")));
        }

        void WriteOnDisk(std::vector<char>&& bytes, std::string&& file_name) {
            std::lock_guard lk{ m_ };
            LOG_INFO("Start writing "s.append(std::to_string(bytes.size()).append(" bytes")));
            net::dispatch(ioc_, [self = this->shared_from_this()
                , bytes = std::move(bytes), file_name = std::move(file_name)]() mutable {
                    self->file_manager_.WriteInRoot(std::move(bytes), std::move(file_name));
                });
        }

        void SetupConnection(std::shared_ptr<http_domain::Client> client
            , std::string_view port) {
            CleanUpInactiveConnections();
            for (const auto& resourse : resourses_) {
                client->Connect(resourse, port);
                if (client->IsConnected()) {
                    resourse_to_clients_[resourse].push_back(client);
                }
            }
            //throw std::runtime_error("All resourses unavailable");
        }

        void SetupNonSecuredConnection() {
            auto client = std::make_shared<http_domain::Client>(ioc_);
            SetupConnection(client, http_domain::Port::NON_SECURED);

        }

        void SetupSecuredConnection() {
            auto client = std::make_shared<http_domain::Client>(ioc_, *ctx_);
            SetupConnection(client, http_domain::Port::SECURED);
        }

        void CheckResoursesForEmpty() {
            if (resourses_.empty()) {
                throw std::runtime_error("Empty resourses");
            }
        }

        void CleanUpInactiveConnections() {
            for (auto& [resourse, clients] : resourse_to_clients_) {
                for (size_t i = 0; i < clients.size();) {
                    if (clients[i]->IsConnected() && !clients[i]->IsBusy()) {
                        clients[i].reset();
                        clients[i] = clients.back();
                        clients.pop_back();
                        continue;
                    }
                    else {
                        ++i;
                    }
                }
            }
        }

    };

}