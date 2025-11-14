#include "../../src/http_client.h"
#include "../../src/connection.h"
#include "../../src/ca_sertificates_loader.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

namespace test_http_client {

    void TestSSLConnect(std::shared_ptr<http::Client> client) {
        client->Connect("httpbin.org", "443");
    }

    void TestConnect(std::shared_ptr<http::Client> client) {
        client->Connect("httpbin.org", "80");
    }

    void RunTests() {
        boost::asio::io_context ioc;
        auto ctx = connection::GetSSLContext();
        auto read_strand = boost::asio::make_strand(ioc);
        auto write_strand = boost::asio::make_strand(ioc);
        {
            auto client = std::make_shared<http::Client>(ioc, *ctx, write_strand, read_strand);
            TestSSLConnect(client);
        }
        {
            auto client = std::make_shared<http::Client>(ioc, write_strand, read_strand);
            TestConnect(client);
        }

    }
}