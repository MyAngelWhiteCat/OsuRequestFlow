#include "../../src/downloader.h"
#include "../../src/file_manager.h"
#include "../../src/connection.h"

#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <boost/asio/io_context.hpp>

namespace test_downloader {
    using namespace std::literals;

    void TestDownloadBytes(std::shared_ptr<downloader::Downloader> downloader, std::string bytes) {
        downloader->Download("/bytes/" + bytes);
    }

    void TestDownload1024bytes(std::shared_ptr<downloader::Downloader> downloader) {
        TestDownloadBytes(downloader, "1024");
    }

    void TestDownload10240bytes(std::shared_ptr<downloader::Downloader> downloader) {
        TestDownloadBytes(downloader, "10240");
    }

    void TestDownload102400bytes(std::shared_ptr<downloader::Downloader> downloader) {
        TestDownloadBytes(downloader, "102400");
    }

    void TestDownloadIevanPolka(std::shared_ptr<downloader::Downloader> downloader) {
        downloader->Download("1886002");
    }

    void RunOsuMapDownloadTest(boost::asio::io_context& ioc) {
        auto ctx = connection::GetSSLContext();
        std::string resource = "catboy.best";
        auto downloader = std::make_shared<downloader::Downloader>(ioc, ctx);
        downloader->SetResourceAndPrefix(resource, "/d/");
        downloader->SetDownloadsDirectory(std::filesystem::current_path().string() + "/downloads");
        TestDownloadIevanPolka(downloader);
    }

}