#include "auth_data.h"
#include "irc_client.h"
#include "logging.h"
#include "connection.h"
#include "downloader.h"


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

    auto ctx_dl = connection::GetSSLContext();
    std::string resourse = "osu.direct";
    auto downloader = std::make_shared<downloader::Downloader>(ioc, ctx_dl);
    downloader->SetResourse(resourse);
    downloader->SetUriPrefix("/api/d/");
    downloader->SetDownloadsFolder(std::filesystem::current_path().string() + "/downloads");
    auto executor = std::make_shared<commands::CommandExecutor>(downloader);
    auto parser = std::make_shared<commands::CommandParser>(*executor);
    auto chat_bot = std::make_shared<chat_bot::ChatBot>(executor, parser);
    auto client = std::make_shared<Client<chat_bot::ChatBot>>(ioc, chat_bot, true);

    std::vector<std::string_view> streamers{"myangelwhitecat"};
    domain::AuthorizeData a_data;

    client->Connect();
    client->Authorize(a_data);
    client->Read();
    client->CapRequest();
    client->Join(streamers);

    LOG_DEBUG("System start...");

    RunWorkers(2, [&ioc]() {
        ioc.run();
        });
    logging::Logger::Shutdown();
}

