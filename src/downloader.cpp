#include "downloader.h"
#include "file_manager.h"
#include "http_client.h"
#include "connection.h"

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
    using ResourcesAccess = std::unordered_map<std::string, std::vector<std::shared_ptr<http_domain::Client>>>;


    Downloader::Downloader(net::io_context& ioc, bool secured)
        : ioc_(ioc)
        , secured_(secured)
        , dl_strand(net::make_strand(ioc))
    {
    }

    Downloader::~Downloader() { // Debug only
        LOG_INFO("Downloader destructed");
    }

    void Downloader::Download(std::string_view file) {
        LOG_INFO("Downdload "s.append(file));
        net::dispatch(dl_strand, [self = this->shared_from_this(), file = std::string(file)]() {
            std::shared_ptr<http_domain::Client> client = nullptr;
            if (self->secured_) {
                client = self->SetupSecuredConnection();
            }
            else {
                client = self->SetupNonSecuredConnection();
            }
            client->SetMaxFileSize(self->max_file_size_MiB_);
            try {
                client->Get(self->GetEndpoint(file), self->user_agent_, [self, client]
                (std::string&& file_name, std::vector<char>&& body) {
                        self->OnDownload(std::move(file_name), std::move(body));
                    });
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
            }
            });
            
    }

    void Downloader::SetUserAgent(std::string_view user_agent) {
        user_agent_ = std::string(user_agent);
    }

    void Downloader::SetUriPrefix(std::string_view uri_prefix) {
        uri_prefix_ = std::string(uri_prefix);
    }

    void Downloader::SetResource(std::string_view resource) {
        resource_ = std::string(resource);
    }

    void Downloader::SetDownloadsDirectory(std::string_view path) {
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

    std::shared_ptr<http_domain::Client> Downloader::SetupNonSecuredConnection() {
        if (!resource_) {
            throw std::runtime_error("Resource doesnt setted");
        }
        auto client = std::make_shared<http_domain::Client>(ioc_);
        client->Connect(*resource_, http_domain::Port::NON_SECURED);
        return client;
    }

    std::shared_ptr<http_domain::Client> Downloader::SetupSecuredConnection() {
        if (!resource_) {
            throw std::runtime_error("Resource doesnt setted");
        }
        auto client = std::make_shared<http_domain::Client>(ioc_, connection::GetSSLContext());
        client->SetMaxFileSize(max_file_size_MiB_);
        client->Connect(*resource_, http_domain::Port::SECURED);
        return client;
    }

    void Downloader::SetMaxFileSize(size_t MiB) {
        max_file_size_MiB_ = MiB;
    }

    std::optional<std::string> Downloader::GetResource() const {
        return resource_;
    }

    std::optional<std::string> Downloader::GetPrefix() const {
        return uri_prefix_;
    }

    size_t Downloader::GetMaxFileSize() const {
        return max_file_size_MiB_;
    }

    std::optional<std::filesystem::path> Downloader::GetDownloadsDirectory() const {
        if (file_manager_) {
            return file_manager_->GetRootDirectory();
        }
        return std::nullopt;
    }

    std::string Downloader::GetUserAgent() const {
        return user_agent_;
    }

    std::string Downloader::GetEndpoint(std::string_view file) {
        if (!uri_prefix_) {
            throw std::runtime_error("prefix doesnt setted");
        }
        return *uri_prefix_ + std::string(file);
    }

} // namespace downloader