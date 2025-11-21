#include "auth_data.h"
#include "client.h"
#include "logging.h"
#include "connection.h"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/impl/io_context.ipp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>

#include <clocale>

#include <memory>
#include <thread>
#include <vector>
#include <string_view>
#include <exception>

using namespace irc;
namespace fs = std::filesystem;

static void ConnectAndReadMultichat(const std::vector<std::string_view>& channels_names
    , const domain::AuthorizeData& a_data
    , std::shared_ptr<Client> client) {
    client->Connect();
    client->CapRequest();
    client->Authorize(a_data);

    client->Join(channels_names);
    try {
        client->Read();
    }
    catch (const std::exception& e) {
        LOG_ERROR(e.what());
    }
    catch (...) { 
        LOG_FUCKUP("call the exorcist! NOW!!!");
    }

    //client->Part(channels_names);
}

template <typename Fn>
void RunWorkers(unsigned num_workers, Fn&& func) {
    try {
        std::vector<std::jthread> threads;
        for (unsigned i = 0; i < num_workers - 1; ++i) {
            threads.emplace_back(func);
        }
        func();
    }
    catch (const std::exception& e) {
        LOG_INFO("Catch exception in RunWorkers");
        LOG_CRITICAL(e.what());
    }
}


int main() {

    logging::Logger::Init();

    setlocale(LC_ALL, "Russian_Russia.1251");

    net::io_context ioc(2);
    auto ctx = connection::GetSSLContext();

    auto work = net::make_work_guard(ioc);

    auto irc_strand = net::make_strand(ioc);

    domain::AuthorizeData a_data; 

    //auto client = std::make_shared<Client>(ioc);
    auto ssl_client = std::make_shared<Client>(ioc, ctx); 

    std::vector<std::string_view> streamers{"fugu_fps"};

    LOG_DEBUG("System start...");
    net::post(irc_strand, [&streamers, &a_data, &ssl_client]() {
        ConnectAndReadMultichat(streamers, a_data, ssl_client);
        });
    RunWorkers(2, [&ioc]() {
        ioc.run();
        });
    logging::Logger::Shutdown();
}

