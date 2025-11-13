#pragma once

#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

#include "connection.h"
#include "file_manager.h"

namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;


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