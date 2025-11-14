#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "file_manager.h"
#include "random_user_agent.h"
#include "http_client.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class Downloader : public std::enable_shared_from_this<Downloader> {

    public:
        Downloader(net::io_context& ioc)
            : user_agent_(100)
            , 
        {
            
        }

        Downloader(net::io_context& ioc, std::shared_ptr<ssl::context> ctx)
            : user_agent_(100)
            , 
        {

        }

        void Download(std::string_view host, std::string_view port, bool secured = true) {
            
        }

    private:
        RandomUserAgent user_agent_;
        http::Client http_client;
    };

}