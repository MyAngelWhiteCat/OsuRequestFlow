#include "auth_data.h"
#include "irc_client.h"
#include "logging.h"
#include "connection.h"
#include "downloader.h"
#include "core.h"


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




int main() {
    logging::Logger::Init();
    setlocale(LC_ALL, "Russian_Russia.1251");

    std::string resourse = "osu.direct";
    std::string streamer = "myangelwhitecat";
    std::string uri_prefix = "/api/d/";
    std::string downloads_path = std::filesystem::current_path().string() + "/downloads";

    core::Core core(2);
    core.SetupConnection(true);
    core.SetupDownloader(true, resourse, uri_prefix, downloads_path);
    core.SetupChatBot();
    core.SetupIRCClient(true);

    core.Start(streamer);
    core.Run(2);
    logging::Logger::Shutdown();
}

