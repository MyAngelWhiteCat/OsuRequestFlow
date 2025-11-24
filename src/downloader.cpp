#include "downloader.h"
#include "file_manager.h"
#include "http_client.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <unordered_map>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/context.hpp>
#include <stdexcept>
#include "logging.h"


namespace downloader {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;
    using ResoursesAccess = std::unordered_map<std::string, std::vector<std::shared_ptr<http_domain::Client>>>;


    Downloader::Downloader(net::io_context& ioc
        , std::shared_ptr<ssl::context> ctx)
        : ioc_(ioc)
        , ctx_(ctx)
    {
    }

    Downloader::Downloader(net::io_context& ioc)
        : ioc_(ioc)
    {
    }

    Downloader::~Downloader() { // Debug only
        LOG_INFO("Downloader destructed");
    }

    void Downloader::Download(std::string_view uri) {
        LOG_INFO("Downdload "s.append(uri));
        if (!client_ || !client_->IsConnected()) {
            if (ctx_) {
                SetupSecuredConnection();
            }
            else {
                SetupNonSecuredConnection();
            }
        }
        client_->Get(GetEndpoint(uri), user_agent_, [self = this->shared_from_this()]
        (std::string&& file_name, std::vector<char>&& body) {
                self->OnDownload(std::move(file_name), std::move(body));
            });
    }

    void Downloader::SetUserAgent(std::string_view user_agent) {
        user_agent_ = std::string(user_agent);
    }

    void Downloader::SetUriPrefix(std::string_view uri_prefix) {
        uri_prefix_ = std::string(uri_prefix);
    }

    void Downloader::SetResourse(std::string_view resourse) {
        resourse_ = std::string(resourse);
    }

    void Downloader::SetDownloadsFolder(std::string_view path) {
        file_manager_ = std::make_shared<file_manager::FileManager>(std::filesystem::path(path));
    }

    void Downloader::OnDownload(std::string&& file_name, std::vector<char>&& body) {
        LOG_INFO("Successfuly download "s.append(std::to_string(body.size())).append(" bytes"));
        WriteOnDisk(std::move(file_name), std::move(body));
    }

    void Downloader::WriteOnDisk(std::string&& file_name, std::vector<char>&& bytes) {
        LOG_INFO("Start writing "s.append(std::to_string(bytes.size()).append(" bytes")));
        net::post([self = this->shared_from_this() //does not require consistent execution btw
            , bytes = std::move(bytes), file_name = std::move(file_name)]() mutable {
                self->file_manager_->WriteBinaryInRoot(std::move(file_name), std::move(bytes));
            });
    }

    void Downloader::SetupNonSecuredConnection() {
        client_ = std::make_shared<http_domain::Client>(ioc_);
        client_->Connect(resourse_, http_domain::Port::NON_SECURED);
    }

    void Downloader::SetupSecuredConnection() {
        client_ = std::make_shared<http_domain::Client>(ioc_, *ctx_);
        client_->Connect(resourse_, http_domain::Port::SECURED);
    }

    std::string Downloader::GetEndpoint(std::string_view file) {
        return uri_prefix_ + std::string(file);
    }

} // namespace downloader