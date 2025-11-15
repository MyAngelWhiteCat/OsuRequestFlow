#include "../../src/http_client.h"

#include "../../src/connection.h"
#include "../../src/ca_sertificates_loader.h"
#include "../../src/random_user_agent.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

namespace test_http_client {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace ssl = net::ssl;

    using net::ip::tcp;
    using namespace std::literals;

    using StringResponse = http::response<http::string_body>;
    using StringRequest = http::request<http::string_body>;
    using DynamicResponse = http::response<http::dynamic_body>;

    void TestSSLConnect(std::shared_ptr<http_domain::Client> client) {
        client->Connect("httpbin.org", "443");

    }

    void TestConnect(std::shared_ptr<http_domain::Client> client) {
        client->Connect("httpbin.org", "80");
    }

    void TestSendRequest(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);
        auto req = http_domain::MakeRequest(boost::beast::http::verb::get
            , "/d/1886002"
            , 11
            , "catboy.best"
            , u_agent.GetUserAgent()
            , "*/*"
            , "close");

        auto handler = [](DynamicResponse&& response) {
            LOG_INFO("Get response: ");
            for (const auto& header : response) {
                std::cout << header.name_string() << ' ' << header.value() << "\n";
            }

            std::string body = beast::buffers_to_string(response.body().data());
            LOG_INFO(body);
            };


        client->Connect("catboy.best", "80");
        client->SendRequest(std::move(req), handler);
    }

    void TestSSLSendRequest(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);
        auto req = http_domain::MakeRequest(boost::beast::http::verb::get
            , "/d/1886002"
            , 11
            , "catboy.best"
            , u_agent.GetUserAgent()
            , "*/*"
            , "close");

        auto handler = [](DynamicResponse&& response) {
            LOG_INFO("Get response: ");
            for (const auto& header : response) {
                std::cout << header.name_string() << ' ' << header.value() << "\n";
            }
            
            std::string body = beast::buffers_to_string(response.body().data());
            LOG_INFO(body);
            };
            

        client->Connect("catboy.best", "443");
        client->SendRequest(std::move(req), handler);
    }

    void TestSSLGet(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);
        auto req = http_domain::MakeRequest(boost::beast::http::verb::get
            , "/d/1886002"
            , 11
            , "catboy.best"
            , u_agent.GetUserAgent()
            , "*/*"
            , "close");

        auto handler = [](DynamicResponse&& response) {
            LOG_INFO("Get response: ");
            for (const auto& header : response) {
                std::cout << header.name_string() << ' ' << header.value() << "\n";
            }

            std::string body = beast::buffers_to_string(response.body().data());
            LOG_INFO(body);
            };

        client->Connect("catboy.best", "443");
        client->Get("/d/1886002", u_agent.GetUserAgent(), handler);
    }

    void RunTests(net::io_context& ioc) {
        auto ctx = connection::GetSSLContext();

        auto client = std::make_shared<http_domain::Client>(ioc);
        auto ssl_client = std::make_shared<http_domain::Client>(ioc, *ctx);

        TestSSLGet(ssl_client);
    }
}