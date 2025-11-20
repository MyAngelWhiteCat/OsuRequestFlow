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
            , Strand& stream_strand)
            : file_write_strand_(stream_strand)
            , ioc_(ioc)
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
            , file_manager::FileManager file_manager
            , Strand& stream_strand)
            : file_write_strand_(stream_strand)
            , ioc_(ioc)
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
            , file_manager::FileManager file_manager
            , Strand& stream_strand)
            : file_write_strand_(stream_strand)
            , ioc_(ioc)
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
            , file_manager::FileManager file_manager
            , Strand& stream_strand)
            : file_write_strand_(stream_strand)
            , ioc_(ioc)
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
                bool has_broken_connections = false;
                for (auto& client : clients) {
                    if (!client->IsConnected()) {
                        has_broken_connections = true;
                        continue;
                    }
                    if (!client->IsBusy()) {
                        client->Get(uri, user_agent_, [self = this->shared_from_this()]
                        (std::string&& file_name, std::vector<char>&& body) {
                                self->OnDownload(std::move(file_name), std::move(body));
                            });
                        return;
                    }
                }
                if (has_broken_connections) {
                    CleanUpInactiveConnections();
                }
                if (resourse_to_clients_.at(resourse).size() < MAX_CONNECTIONS) {

                }
            }
        }


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

        void OnDownload(std::string&& file_name, std::vector<char>&& body) {
            LOG_INFO("Successfuly download "s.append(std::to_string(body.size())).append(" bytes"));
            LOG_INFO("Headers:");
            WriteOnDisk(std::move(file_name), std::move(body));
        }

        void WriteOnDisk(std::string&& file_name, std::vector<char>&& bytes) {
            LOG_INFO("Start writing "s.append(std::to_string(bytes.size()).append(" bytes")));
            //net::post(file_write_strand_, [self = this->shared_from_this() //does not require consistent execution btw
                //, bytes = std::move(bytes), file_name = std::move(file_name)]() mutable {
                    /*self->*/file_manager_.WriteInRoot(std::move(file_name), std::move(bytes));
                //});
        }

        void SetupConnection(std::shared_ptr<http_domain::Client> client
            , std::string_view port) {
            CleanUpInactiveConnections();
            for (const auto& resourse : resourses_) {
                if (ConnectToResourse(client, resourse, port)) {
                    return;
                }
            }
            throw std::runtime_error("All resourses unavailable");
        }

        bool ConnectToResourse(std::shared_ptr<http_domain::Client> client
            , std::string_view resourse, std::string_view port) {
            client->Connect(resourse, port);
            if (client->IsConnected()) {
                resourse_to_clients_[std::string(resourse)].push_back(client);
                return true;
            }
            return false;
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