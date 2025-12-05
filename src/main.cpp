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
    logging::Logger::Init();
    setlocale(LC_ALL, "Russian_Russia.1251");

    net::io_context ioc;
    LOG_INFO("Carefully look at this path ");
    LOG_INFO(fs::current_path().string());
    LOG_INFO("If this path have something like йцукенгшщзхъфывапролджячсмитьбю");
    LOG_INFO("replace .exe to eng only folders path");
    LOG_INFO("if path is OK Press ENTER.");

    std::cin.get();

    core::Core core(ioc);
    core.SetupConnection(true);
    core.SetupDownloader(true);
    core.SetupIRCClient(true);
    core.Start();
    core.LoadSettings();

    auto address = net::ip::make_address("127.0.0.1");
    boost::asio::ip::tcp::endpoint localhost{ address, 23140 };
    fs::path root = fs::current_path() / "../static";

    
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

