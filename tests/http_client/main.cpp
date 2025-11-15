#include "test_http_client.h"

int main() {
    try {
        boost::asio::io_context ioc;
        test_http_client::RunTests(ioc);
        ioc.run();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL(e.what());
    }
}