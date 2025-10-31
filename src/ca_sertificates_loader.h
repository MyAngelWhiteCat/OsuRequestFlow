#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <openssl/x509.h>
#pragma comment(lib, "crypt32.lib")
#endif

namespace ssl_domain_utilities {

    namespace ssl = boost::asio::ssl;

    void load_windows_ca_certificates(ssl::context& ctx);


}

