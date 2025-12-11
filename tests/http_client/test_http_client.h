#include "../../src/http_client.h"

#include "../../src/connection.h"
#include "../../src/ca_sertificates_loader.h"
#include "../../src/random_user_agent.h"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <fstream>

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


    auto handler = [](http_domain::DLMetaData&& metadata) {
        LOG_INFO("Get response");
        std::ofstream out("downloads/" + metadata.file_name_);
        LOG_INFO("File writen");
        };

    RandomUserAgent gu_agent(100);
    http_domain::RequestBuilder req_builder;
    auto req = req_builder.MakeRequest(boost::beast::http::verb::get
        , "/d/1886002"
        , 11
        , "catboy.best"
        , gu_agent.GetUserAgent()
        , "*/*"
        , "close");

    void TestSSLConnect(std::shared_ptr<http_domain::Client> client) {
        client->Connect("httpbin.org", "443");
    }

    void TestConnect(std::shared_ptr<http_domain::Client> client) {
        client->Connect("httpbin.org", "80");
    }

    void TestSendRequest(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);

        client->Connect("catboy.best", "80");
        client->SendRequest(std::move(req), handler);
    }

    void TestSSLSendRequest(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(60);

        client->Connect("catboy.best", "443");
        client->SendRequest(std::move(req), handler);
    }
    //https://osu.direct/api/d/1886002 
    //https://catboy.best/d/1886002 

    void TestGet(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);

        client->Connect("catboy.best", "443");
        client->Get("/d/1886002", u_agent.GetUserAgent(), handler);
    }

    void TestSSLGet(std::shared_ptr<http_domain::Client> client) {
        RandomUserAgent u_agent(100);

        client->Connect("osu.direct", "443");
        client->Get("/api/d/1886002", u_agent.GetUserAgent(), handler);
    }

    void RunTests(net::io_context& ioc) {
        auto ctx = connection::GetSSLContext();

        auto client = std::make_shared<http_domain::Client>(ioc);
        auto ssl_client = std::make_shared<http_domain::Client>(ioc, ctx);

        //TestSSLGet(ssl_client);
        //TestSSLSendRequest(ssl_client);
    }
}