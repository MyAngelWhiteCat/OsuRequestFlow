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
    , std::shared_ptr<Client<handler::MessageHandler>> client) {
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

int main() {
    logging::Logger::Init();
    setlocale(LC_ALL, "Russian_Russia.1251");
    net::io_context ioc;
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

    auto client = std::make_shared<Client<handler::MessageHandler>>(ioc, irc_strand);
    auto ssl_client = std::make_shared<Client<handler::MessageHandler>>(ioc, ctx, irc_strand); // ponatno po nazvaniyam

    std::vector<std::string_view> streamers{ "Topson", "shroud", "summit1g" };

    LOG_INFO("System start...");
    net::post(irc_strand, [&streamers, &a_data, &ssl_client]() {
        ConnectAndReadMultichat(streamers, a_data, ssl_client);
        });

    ioc.run();
}

