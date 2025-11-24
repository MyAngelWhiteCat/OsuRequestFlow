#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include <boost/beast.hpp>
#include <string_view>
#include <string>

#include "logging.h"
#include "request_builder.h"


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


    StringRequest http_domain::RequestBuilder::MakeRequest(http::verb method, std::string_view target
        , int version, std::string_view host, std::string_view user_agent, std::string_view accept
        , std::string_view connection) {
        StringRequest req;
        req.method(method);
        req.target(target);
        req.version(version);

        req.set(http::field::host, host);
        req.set(http::field::user_agent, user_agent);
        req.set(http::field::accept, accept);
        req.set(http::field::connection, connection);
        req.set(http::field::accept_encoding, "gzip, deflate, br");

        return req;
    }

    StringRequest RequestBuilder::Get(std::string_view target, std::string_view user_agent
        , std::string_view host) {
        return MakeRequest(http::verb::get, target, 11, host, user_agent, accept_, connection_);
    }

    StringRequest RequestBuilder::Head(std::string_view target, std::string_view user_agent
        , std::string_view host) {
        return MakeRequest(http::verb::head, target, 11, host, user_agent, accept_, connection_);
    }

}