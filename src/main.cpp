#include "client.h"
#include "ca_sertificates_loader.h"
#include "message_handler.h"
#include "message.h"
#include "auth_data.h"
#include "logging.h"

#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>

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
    std::vector<std::jthread> threads;
    for (unsigned i = 0; i < num_workers - 1; ++i) {
        threads.emplace_back(func);
    }
    func();
}

int main() {
    logging::Logger::Init();

    setlocale(LC_ALL, "Russian_Russia.1251");
    net::io_context ioc(2);
    auto work = net::make_work_guard(ioc);
    ssl::context ctx{ ssl::context::tlsv12_client };
    auto irc_strand = net::make_strand(ioc);

    ctx.set_verify_mode(ssl::verify_peer); 
    
    // AI on
    try {
        ctx.set_default_verify_paths(); 
    }
    catch (...) {}
    ssl_domain_utilities::load_windows_ca_certificates(ctx); 
    // AI off

    domain::AuthorizeData a_data; 

    //auto client = std::make_shared<Client>(ioc);
    auto ssl_client = std::make_shared<Client>(ioc, ctx); 

    std::vector<std::string_view> streamers{ "summit1g", "xQc", "CDawg" };

    LOG_DEBUG("System start...");
    net::post(irc_strand, [&streamers, &a_data, &ssl_client]() {
        ConnectAndReadMultichat(streamers, a_data, ssl_client);
        });
    RunWorkers(2, [&ioc]() {
        ioc.run();
        });
    logging::Logger::Shutdown();
}

