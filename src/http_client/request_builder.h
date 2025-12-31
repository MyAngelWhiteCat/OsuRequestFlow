#pragma once 

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast.hpp>
#include <string_view>

#include "logger/logging.h"
#include <string>


namespace http_domain {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace ssl = net::ssl;

    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using StringResponse = http::response<http::string_body>;
    using StringRequest = http::request<http::string_body>;
    using DynamicResponse = http::response<http::dynamic_body>;


    class RequestBuilder {
    public:
        StringRequest MakeRequest(http::verb method, std::string_view target, int version
            , std::string_view host, std::string_view user_agent, std::string_view accept
            , std::string_view connection);

        StringRequest Get(std::string_view target, std::string_view user_agent, std::string_view host);

        StringRequest Head(std::string_view target, std::string_view user_agent, std::string_view host);

    private:
        std::string accept_ = "*/*";
        std::string connection_ = "keep-alive";

    };

}