#pragma once

#include <boost/asio/ssl/context.hpp>

#ifdef _WIN32
#pragma comment(lib, "crypt32.lib")
#endif

namespace ssl_domain_utilities {

    namespace ssl = boost::asio::ssl;

    void load_windows_ca_certificates(ssl::context& ctx);

}

