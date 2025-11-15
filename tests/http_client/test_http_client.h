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

    void TestGet(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);
        auto req = http_domain::MakeRequest(boost::beast::http::verb::get
            , "/get"
            , 11
            , "httpbin.org"
            , u_agent.GetUserAgent()
            , "*/*"
            , "close");

        auto handler = [](DynamicResponse&& response) mutable {
            for (const auto& title : response) {
                LOG_INFO(title.name_string());
            }
            std::string body = beast::buffers_to_string(response.body().data());
            };

        client->Connect("httpbin.org", "80");
        client->SendRequest(std::move(req), handler);
    }

    void TestSSLGet(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);
        auto req = http_domain::MakeRequest(boost::beast::http::verb::get
            , "/get"
            , 11
            , "httpbin.org"
            , u_agent.GetUserAgent()
            , "*/*"
            , "close");

        auto handler = [](DynamicResponse&& response) {
            for (const auto& title : response) {
                LOG_INFO(title.name_string());
            }
            };

        client->Connect("httpbin.org", "443");
        client->SendRequest(std::move(req), handler);
    }

    void RunTests(net::io_context& ioc) {
        auto ctx = connection::GetSSLContext();
        auto read_strand = boost::asio::make_strand(ioc);
        auto write_strand = boost::asio::make_strand(ioc);

        auto client = std::make_shared<http_domain::Client>(ioc, write_strand, read_strand);
        auto ssl_client = std::make_shared<http_domain::Client>(ioc, *ctx, write_strand, read_strand);

        try
        {
            TestGet(client);
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }

        try 
        {
            TestSSLGet(ssl_client);
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }
    }
}