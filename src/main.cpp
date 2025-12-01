#include "auth_data.h"
#include "irc_client.h"
#include "logging.h"
#include "core.h"
#include "http_server.h"
#include "request_handler.h"

#include <boost/asio/impl/io_context.ipp>
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

bool OpenBrowserAtPort8181() {
    std::string url = "http://127.0.0.1:8181";
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
    logging::Logger::Init();
    setlocale(LC_ALL, "Russian_Russia.1251");
    net::io_context ioc;

    core::Core core(ioc);
    core.SetupConnection(true);
    core.SetupDownloader(true);
    core.SetupIRCClient(true);
    core.Start();

    auto address = net::ip::make_address("127.0.0.1");
    boost::asio::ip::tcp::endpoint localhost{ address, 8181 };
    fs::path root = fs::current_path() / "../static";

    LOG_INFO("MAKE SURE YOU DONT HAVE ANY kirilic/arabic/chineese NAMED FOLDERS IN PATH TO THIS EXE");
    LOG_INFO("OR NOW YOU WILL SEE EPIC CRASH WITH 5+ ASSERTION");
    LOG_INFO(fs::current_path().string());
    LOG_INFO("IF THIS PATH DONT HAVE ANY ABRAKADABRA");
    LOG_INFO("Fingerprint anything you want.");
    std::string str;
    std::cin >> str;

    Strand api_strand = net::make_strand(ioc);
    auto handler = std::make_shared<gui_http::RequestHandler>(core, root, api_strand);
    http_server::ServeHttp(ioc, localhost, [handler](auto&& req, auto&& send) mutable {
        (*handler)(std::move(req), std::move(send));
        });

    OpenBrowserAtPort8181();
    LOG_INFO("GO TO http://localhost:8181");
    RunWorkers(6, [&ioc]() {
        ioc.run();
        });
    logging::Logger::Shutdown();
}

