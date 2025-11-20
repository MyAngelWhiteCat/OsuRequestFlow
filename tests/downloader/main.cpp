#include "test_downloader.h"


int main() {
    try {
        boost::asio::io_context ioc;
        test_downloader::RunOsuMapDownloadTest(ioc);
        ioc.run();
    }
    catch (const std::exception& e) {
        LOG_CRITICAL(e.what());
    }
}