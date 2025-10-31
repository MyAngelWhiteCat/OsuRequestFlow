#include "irc.h"
#include "ca_sertificates_loader.h"

#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace irc;
namespace fs = std::filesystem;


auto init = atexit([] {
    std::cout << "BB!" << std::endl;
    });

static void TestConnect(std::string_view channel_name
    , const AuthorizeData& a_data
    , Client& client) {


    std::cout << "Test case - connect. " << "Join " << channel_name << std::endl;
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);
    client.Join(channel_name);
    std::cout << "End of test case - connect. " << std::endl;
    client.Part(channel_name);
    client.Disconnect();
}

static void ConnectAndReadChat(std::string_view channel_name
    , const AuthorizeData& a_data
    , Client& client
    , std::chrono::seconds how_long = 100s
    , bool infinity = false) {


    std::cout << "Test case - connect. And Read Chat " << "Join " << channel_name << std::endl;
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);
    client.Join(channel_name);
    auto start = std::chrono::steady_clock::now();
    while (infinity || std::chrono::steady_clock::now() - start < how_long) {
        try {
            auto read_result = client.Read();
            for (const auto& message : read_result) {
                if (message.GetMessageType() == domain::MessageType::PRIVMSG) {
                    std::cout << message.GetNick() << ": "
                              << message.GetContent() << std::endl;
                }
                else {
                    continue;
                }
            }
        }
        catch (...) { // TODO: Error logging
            std::cout << "Ooops... Something wrong here" << std::endl;
        }
    }

    std::cout << "End of test case - connect. " << std::endl;
    client.Part(channel_name);
    client.Disconnect();
}

static void ConnectAndReadChat(std::string_view channel_name
    , const AuthorizeData& a_data
    , Client& client
    , bool infinity = false) {
    ConnectAndReadChat(channel_name, a_data, client, 0s, true);
}

static void ConnectAndReadMultichat(const std::vector<std::string>& channels_names
    , const AuthorizeData& a_data
    , Client& client
    , std::chrono::seconds how_long = 100s
    , bool infinity = false) {
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);

    for (const auto& channel_name : channels_names) {
        client.Join(channel_name);
    }
    auto start = std::chrono::steady_clock::now();
    while (infinity || std::chrono::steady_clock::now() - start < how_long) {
        try {
            auto read_result = client.Read();
            for (const auto& message : read_result) {
                if (message.GetMessageType() == domain::MessageType::PRIVMSG) {
                    std::cout << message.GetNick() << ": "
                              << message.GetContent() << std::endl;
                }
                else {
                    continue;
                }
            }
        }
        catch (...) { // TODO: Error logging
            std::cout << "Ooops... Something wrong here" << std::endl;
        }
    }
}

static void ConnectAndReadMultichat(const std::vector<std::string>& channels_names
    , const AuthorizeData& a_data
    , Client& client
    , bool infinity) {

    ConnectAndReadMultichat(channels_names, a_data, client, 0s, infinity);

}

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251"); // Chtobi oshibki pisalis' ponytno
    net::io_context ioc; // input-output context.
    ssl::context ctx{ ssl::context::tlsv12_client }; // ssl context
    ctx.set_verify_mode(ssl::verify_peer); // ustanovka urovnya verifikatsyi. Dlya SSl.

    try {
        ctx.set_default_verify_paths(); // eto dlya linux u macos
    }
    catch (...) {}

    ssl_domain_utilities::load_windows_ca_certificates(ctx); // downloading ssl serts dlya windy

    AuthorizeData a_data; // Auth data. For connectiong to twitch
    Client client(ioc);
    Client ssl_client(ioc, ctx); // ponatno po nazvaniyam

  //ConnectAndReadChat("myangelwhitecat", a_data, ssl_client, 20s); // 20 sec pochitaet chat
  //ConnectAndReadChat("myangelwhitecat", a_data, ssl_client, true); // budet chitat' while(true)
  //Mozhno peredat' vremya chteniya v secundah libo true dlya while(true)

    std::vector<std::string> streamers{ "caedrel", "BTMC", "enri", "RiotGames", "jasontheween", "yourragegaming" };
    ConnectAndReadMultichat(streamers, a_data, ssl_client, true); // Srazu chat vseh
}

