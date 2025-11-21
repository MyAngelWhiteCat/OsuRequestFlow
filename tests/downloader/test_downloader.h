#include "../../src/downloader.h"
#include "../../src/random_user_agent.h"
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
        downloader->Download("/d/1886002");
    }

    void RunTests(boost::asio::io_context& ioc) {
        auto ctx = connection::GetSSLContext();
        file_manager::FileManager f_manager(std::filesystem::path(std::filesystem::current_path().string() +  "/downloads"));
        std::vector<std::string> resourses{ "httpbin.org" };
        auto u_agent = std::make_shared<RandomUserAgent>(100);
        auto strand = boost::asio::make_strand(ioc);
        auto downloader = std::make_shared<downloader::Downloader>(ioc, ctx, resourses, u_agent, f_manager, strand);
        //TestDownload1024bytes(downloader);
        //TestDownload10240bytes(downloader);
        TestDownload102400bytes(downloader);
    }

    void RunOsuMapDownloadTest(boost::asio::io_context& ioc) {
        auto ctx = connection::GetSSLContext();
        file_manager::FileManager f_manager(std::filesystem::path(std::filesystem::current_path().string() + "/downloads"));
        std::vector<std::string> resourses{ "catboy.best", "osu.direct" };
        auto strand = boost::asio::make_strand(ioc);

        auto u_agent = std::make_shared<RandomUserAgent>(100);
        auto downloader = std::make_shared<downloader::Downloader>(ioc, ctx, resourses, u_agent, f_manager, strand);
        TestDownloadIevanPolka(downloader);
    }

}