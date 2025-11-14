#pragma once

#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

#include "connection.h"
#include "file_manager.h"

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace ssl = net::ssl;

    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using StringResponse = http::response<http::string_body>;
    using DynamicResponse = http::response<http::dynamic_body>;


    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader()
            
        {
            
        }

        void Download(std::string_view host, std::string_view port, bool secured = true) {

        }

    private:
        
    };

}