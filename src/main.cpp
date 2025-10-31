#include "irc.h"
#include "ca_sertificates_loader.h"

#include <vector>
#include <chrono>


using namespace irc;


static void TestSSLConnect(std::string_view chanel_name
    , const AuthorizeData& a_data
    , net::io_context& ioc
    , ssl::context& ctx) {

    Client client(ioc, ctx);

    std::cout << "Test case - SSL connect. " << "Join " << chanel_name << std::endl;
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);
    client.Join(chanel_name);
    std::cout << "End of test case - SSL connect. " << std::endl;
}

static void TestConnect(std::string_view chanel_name
    , const AuthorizeData& a_data
    , net::io_context& ioc) {

    Client client(ioc);

    std::cout << "Test case - connect. " << "Join " << chanel_name << std::endl;
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);
    client.Join(chanel_name);
    std::cout << "End of test case - connect. " << std::endl;
}

static void TestConnectAndReadChat(std::string_view chanel_name
    , const AuthorizeData& a_data
    , net::io_context& ioc) {

    Client client(ioc);

    std::cout << "Test case - connect. " << "Join " << chanel_name << std::endl;
    client.Connect();
    client.CapRequest();
    client.Authorize(a_data);
    client.Join(chanel_name);
    std::cout << "End of test case - connect. " << std::endl;
    client.Disconnect();
}

int main() {
    //setlocale(LC_ALL, "Russian");
    setlocale(LC_ALL, "Russian_Russia.1251");
    net::io_context ioc;
    ssl::context ctx{ ssl::context::tlsv12_client };
    ctx.set_verify_mode(ssl::verify_peer);

    try {
        ctx.set_default_verify_paths();
    }
    catch (...) {}

    ssl_domain_utilities::load_windows_ca_certificates(ctx);

    AuthorizeData a_data;

    {
        Client client(ioc, ctx);

        std::cout << "Test case #1. With unexpected chat users and messages" << std::endl;
        client.Connect();
        client.CapRequest();
        client.Authorize(a_data);
        client.Join("chicony");

        auto start = std::chrono::steady_clock::now();
        while (/*std::chrono::steady_clock::now() - start < 10000ms*/true) {
            try {
                auto rr = client.Read();
                for (const auto& r : rr) {
                    if (r.GetMessageType() == domain::MessageType::EMPTY) {
                        continue;
                    }
                    if (r.GetMessageType() == domain::MessageType::UNKNOWN) {
                        auto content = r.GetContent();
                        if (!content.empty()) {
                            std::cout << content << std::endl;
                        }
                        if (content.find("\n") != std::string::npos) {
                            std::cout << "Catch this bitch" << std::endl;

                        }
                        continue;
                    }
                    else {
                        continue;
                    }
                    if (r.GetMessageType() == domain::MessageType::PRIVMSG) {
                        std::cout << r.GetNick() << ": " << r.GetContent() << std::endl;
                    }
                    else {
                        domain::PrintMessageType(std::cout, r.GetMessageType());
                        std::cout << ": ";
                        std::cout << r.GetContent() << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }

        client.Disconnect();
    }

    {
        Client client(ioc);
        std::cout << "Test case #2. With expected chat messages" << std::endl;
        client.Connect();
        client.CapRequest();
        client.Authorize(a_data);
        client.Join("myangelwhitecat");

        while (true) {
            try {
                auto rr = client.Read();
                for (const auto& r : rr) {
                    if (r.GetMessageType() == domain::MessageType::PRIVMSG) {
                        std::cout << r.GetNick() << ": " << r.GetContent() << std::endl;
                    }
                    else {
                        domain::PrintMessageType(std::cout, r.GetMessageType());
                        std::cout << ": ";
                        std::cout << r.GetContent() << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
        client.Disconnect();

    }
}

