#include "irc.h"
#include "ca_sertificates_loader.h"
#include "message_handler.h"
#include "message.h"
#include "auth_data.h"

#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace irc;
namespace fs = std::filesystem;


static void ConnectAndReadMultichat(const std::vector<std::string>& channels_names
    , const domain::AuthorizeData& a_data
    , std::shared_ptr<Client<handler::MessageHandler>> client) {
    client->Connect();
    client->CapRequest();
    client->Authorize(a_data);

    for (const auto& channel_name : channels_names) {
        client->Join(channel_name);
    }
    try {
        client->Read();
    }
    catch (...) { // TODO: Error logging
        std::cout << "Ooops... Something wrong here" << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251"); // Chtobi oshibki pisalis' ponytno
    net::io_context ioc; // input-output context.
    ssl::context ctx{ ssl::context::tlsv12_client }; // ssl context
    auto irc_strand = net::make_strand(ioc);

    ctx.set_verify_mode(ssl::verify_peer); // ustanovka urovnya verifikatsyi. Dlya SSl.

    try {
        ctx.set_default_verify_paths(); // eto dlya linux u macos
    }
    catch (...) {}

    ssl_domain_utilities::load_windows_ca_certificates(ctx); // downloading ssl serts dlya windy

    domain::AuthorizeData a_data; // Auth data. For connection to twitch


    auto client = std::make_shared<Client<handler::MessageHandler>>(ioc, irc_strand);
    auto ssl_client = std::make_shared<Client<handler::MessageHandler>>(ioc, ctx, irc_strand); // ponatno po nazvaniyam

    std::vector<std::string> streamers{ "caedrel", "BTMC", "enri", "RiotGames", "jasontheween", "yourragegaming" };
    net::post(irc_strand, [&streamers, &a_data, &ssl_client]() {
        ConnectAndReadMultichat(streamers, a_data, ssl_client);
        });

    ioc.run();
}

