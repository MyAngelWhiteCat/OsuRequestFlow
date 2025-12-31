#include "twitch_irc_client/auth_data.h"
#include "twitch_irc_client/irc_client.h"
#include "logger/logging.h"
#include "core/core.h"
#include "http_server/http_server.h"
#include "request_handler.h"

#include <boost/asio/io_context.hpp>

#include <clocale>

#include <thread>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include <Lmcons.h>
#include <filesystem>

using namespace irc;
namespace fs = std::filesystem;

namespace net = boost::asio;
namespace sys = boost::system;
namespace ssl = net::ssl;
using net::ip::tcp;
using namespace std::literals;

using Strand = net::strand<net::io_context::executor_type>;

template <typename Fn>
void RunWorkers(unsigned num_workers, const Fn& fn) {
    std::vector<std::jthread> threads;
    for (unsigned i = 0; i < num_workers - 1; ++i) {
        threads.emplace_back(fn);
    }
    fn();
}

void RemoveOldLogs() {
    fs::path log_access = fs::current_path() / "LogAccessTestResult.txt";
    if (fs::exists(log_access)) {
        fs::remove(log_access);
    }
    fs::path log_request = fs::current_path() / "LogRequest.txt";
    if (fs::exists(log_request)) {
        fs::remove(log_request);
    }
}

bool OpenBrowserAtPort23140() {
    std::string url = "http://127.0.0.1:23140";
    HINSTANCE result = ShellExecuteA(
        nullptr,
        "open",
        url.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
    return reinterpret_cast<int>(result) > 32;
}

int main() {
    RemoveOldLogs();

    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    setlocale(LC_ALL, "Russian_Russia.1251");
    std::ios_base::sync_with_stdio(false);
    std::locale loc("Russian_Russia.1251");
    std::cout.imbue(loc);
    std::cerr.imbue(loc);
    std::cin.imbue(loc);

    logging::Logger::Init();

    net::io_context ioc(6);
    LOG_CRITICAL("Функции белых и черных списков не проходили полноценного тестирования в связи с чем не гарантируют идеальной работы.");
    LOG_INFO(fs::current_path().string());
    LOG_INFO("Убедитесь, что путь к RequestFlow.exe (для удобства написан выше) не содержит русских имен");
    bool found_static = false;
    for (const auto& dir_entry : fs::directory_iterator(fs::current_path())) {
        if (dir_entry.is_directory()) {
            std::string path_ = dir_entry.path().string();
            std::string folder_name;
            folder_name = path_.substr(path_.find_last_of('\\') + 1);
            if (folder_name == "static") {
                found_static = true;
            }
        }
    }
    if (!found_static) {
        LOG_CRITICAL("Папка static не найдена!\n");
        LOG_INFO("Нажмите Enter для выхода...");
        std::cin.get();
        return -2;
    }
    LOG_INFO("if path is OK Press ENTER.");

    std::cin.get();

    core::Core core(ioc);
    core.SetupConnection(true);
    core.SetupDownloader(true);
    core.SetupIRCClient(true);
    core.Start();
    core.LoadSettings();
    //core.MesureDownloadSpeed();

    auto address = net::ip::make_address("127.0.0.1");
    boost::asio::ip::tcp::endpoint localhost{ address, 23140 };
    fs::path root = fs::current_path() / "static";

    Strand api_strand = net::make_strand(ioc);
    auto handler = std::make_shared<gui_http::RequestHandler>(core, root, api_strand);
    http_server::ServeHttp(ioc, localhost, [handler](auto&& req, auto&& send) mutable {
        (*handler)(std::move(req), std::move(send));
        });

    OpenBrowserAtPort23140();
    LOG_INFO("GO TO http://localhost:23140");
    RunWorkers(6, [&ioc]() {
        ioc.run();
        });
    logging::Logger::Shutdown();
}
