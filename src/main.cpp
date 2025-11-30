#include "auth_data.h"
#include "irc_client.h"
#include "logging.h"
#include "downloader.h"
#include "core.h"
#include "http_server.h"
#include "request_handler.h"

#include <boost/asio/impl/io_context.ipp>
#include <boost/asio/io_context.hpp>

#include <clocale>

#include <thread>
#include <vector>

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

int main() {
    logging::Logger::Init();
    setlocale(LC_ALL, "Russian_Russia.1251");

    std::string resourse = "osu.direct";
    std::string uri_prefix = "/api/d/";
    std::string downloads_path = std::filesystem::current_path().string() + "/downloads";

    net::io_context ioc;

    core::Core core(ioc);
    core.SetupConnection(true);
    core.SetupDownloader(true, resourse, uri_prefix, downloads_path);
    core.SetupChatBot();
    core.SetupIRCClient(true);
    core.Start();

    auto address = net::ip::make_address("127.0.0.1");
    boost::asio::ip::tcp::endpoint localhost{ address, 8181 };
    std::filesystem::path root = std::filesystem::current_path() / "../static";

    Strand api_strand = net::make_strand(ioc);
    auto handler = std::make_shared<gui_http::RequestHandler>(core, root, api_strand);
    http_server::ServeHttp(ioc, localhost, [handler](auto&& req, auto&& send) mutable {
        (*handler)(std::move(req), std::move(send));
        });

    RunWorkers(6, [&ioc]() {
        ioc.run();
        });

    logging::Logger::Shutdown();
}

