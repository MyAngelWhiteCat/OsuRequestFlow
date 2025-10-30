#include "irc.h"

#include <vector>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <openssl/x509.h>
#pragma comment(lib, "crypt32.lib")


using namespace irc;

void load_windows_ca_certificates(ssl::context& ctx) {
    HCERTSTORE hStore = CertOpenSystemStoreA(NULL, "ROOT");
    if (!hStore) {
        return;
    }

    PCCERT_CONTEXT pContext = nullptr;
    while ((pContext = CertEnumCertificatesInStore(hStore, pContext))) {
        const unsigned char* cert_data = pContext->pbCertEncoded;
        X509* x509 = d2i_X509(nullptr, &cert_data, pContext->cbCertEncoded);
        if (x509) {
            X509_STORE* store = SSL_CTX_get_cert_store(ctx.native_handle());
            if (store) {
                X509_STORE_add_cert(store, x509);
            }
            X509_free(x509);
        }
    }

    CertCloseStore(hStore, 0);
}
#else

void load_windows_ca_certificates(ssl::context& ctx) {
    (void)ctx;
}
#endif

std::string convert_utf8_to_ansi(const std::string& utf8_str) {
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    wchar_t* wide_str = new wchar_t[wide_len];
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wide_str, wide_len);

    int ansi_len = WideCharToMultiByte(CP_ACP, 0, wide_str, -1, nullptr, 0, nullptr, nullptr);
    char* ansi_str = new char[ansi_len];
    WideCharToMultiByte(CP_ACP, 0, wide_str, -1, ansi_str, ansi_len, nullptr, nullptr);

    std::string result(ansi_str);
    delete[] wide_str;
    delete[] ansi_str;
    return result;
}

int main() {
    //setlocale(LC_ALL, "Russian");
    setlocale(LC_ALL, "Russian_Russia.1251");
    net::io_context ioc;
    ssl::context ctx{ ssl::context::tlsv12_client };
    ctx.set_verify_mode(ssl::verify_peer);

    try {
        ctx.set_default_verify_paths();
    }
    catch (...) {}

    load_windows_ca_certificates(ctx);

    AuthorizeData a_data;

    {
        Client client(ioc, ctx);

        std::cout << "Test case #1. With unexpected chat users and messages" << std::endl;
        client.Connect();
        client.CapRequest();
        client.Authorize(a_data);
        client.Join("chicony");

        auto start = std::chrono::steady_clock::now();
        while (/*std::chrono::steady_clock::now() - start < 10000ms*/true) {
            try {
                auto rr = client.Read();
                for (const auto& r : rr) {
                    if (r.GetMessageType() == domain::MessageType::EMPTY) {
                        continue;
                    }
                    if (r.GetMessageType() == domain::MessageType::UNKNOWN) {
                        auto content = r.GetContent();
                        if (!content.empty()) {
                            std::cout << content << std::endl;
                        }
                        continue;
                    }
                    else {
                        continue;
                    }
                    if (r.GetMessageType() == domain::MessageType::PRIVMSG) {
                        std::cout << r.GetNick() << ": " << r.GetContent() << std::endl;
                    }
                    else {
                        domain::PrintMessageType(std::cout, r.GetMessageType());
                        std::cout << ": ";
                        std::cout << r.GetContent() << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }

        client.Disconnect();
    }

    {
        Client client(ioc);
        std::cout << "Test case #2. With expected chat messages" << std::endl;
        client.Connect();
        client.CapRequest();
        client.Authorize(a_data);
        client.Join("myangelwhitecat");

        while (true) {
            try {
                auto rr = client.Read();
                for (const auto& r : rr) {
                    if (r.GetMessageType() == domain::MessageType::PRIVMSG) {
                        std::cout << r.GetNick() << ": " << r.GetContent() << std::endl;
                    }
                    else {
                        domain::PrintMessageType(std::cout, r.GetMessageType());
                        std::cout << ": ";
                        std::cout << r.GetContent() << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
        client.Disconnect();

    }
}

