#include "client.h"

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "auth_data.h"
#include "connection.h"
#include "domain.h"
#include "logging.h"
#include "message_handler.h"
#include "message_processor.h"


namespace irc {

    Client::Client(net::io_context& ioc, std::shared_ptr<ssl::context> ctx)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_strand_(net::make_strand(ioc))
        , connection_(std::make_shared<connection::Connection>(ioc, *ctx, write_strand_, read_strand_))
        , ctx_(ctx)
        , reconnect_timer_(ioc)
    {
    }

    Client::Client(net::io_context& ioc)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_strand_(net::make_strand(ioc))
        , connection_(std::make_shared<connection::Connection>(ioc, write_strand_, read_strand_))
        , reconnect_timer_(ioc)
    {

    }

    void Client::Connect() {
        connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
    }

    void Client::Disconnect() {
        connection_->Disconnect();
    }

    void Client::Join(const std::vector<std::string_view>& channels_names) {
        join_command_buffer_ = GetChannelNamesInStringCommand(channels_names);
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + *join_command_buffer_ + "\r\n"s);
    }

    void Client::Join() {
        if (!join_command_buffer_) {
            throw std::runtime_error("Empty reconnect buffer");
        }
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + *join_command_buffer_ + "\r\n"s);
    }

    void Client::Part(const std::vector<std::string_view>& channels_names) {
        std::string command = GetChannelNamesInStringCommand(channels_names);
        connection_->Write(std::string(domain::Command::PART_CHANNEL) + command + "\r\n"s);
    }

    void Client::Authorize() {
        if (!auth_data_buffer_) {
            throw std::runtime_error("Empty reconnect buffer");
        }
        connection_->Write(*auth_data_buffer_);
    }


    void Client::Authorize(const domain::AuthorizeData& auth_data) {
        auth_data_buffer_ = auth_data.GetAuthMessage();
        connection_->Write(*auth_data_buffer_);
    }

    void Client::CapRequest() {
        connection_->Write(std::string(domain::Command::CREQ)
            + std::string(domain::Capabilityes::COMMANDS) + " "
            + std::string(domain::Capabilityes::MEMBERSHIP) + " "
            + std::string(domain::Capabilityes::TAGS) + "\r\n");
    }

    void Client::Read() {
        auto process_message = net::bind_executor(read_strand_, [self = shared_from_this()](std::vector<char>&& bytes) {
                self->OnRead(std::move(bytes));
                });
        connection_->AsyncRead(process_message);
    }

    //TODO
    bool Client::CheckConnect() {
        return connection_->IsConnected();
    }

    void Client::SetReconnectTimeout(int timeout) {
        reconnect_timeout_ = timeout;
    }

    void Client::OnRead(std::vector<char>&& bytes) {
        try {
            std::vector<char> saved_bytes = std::move(bytes);
            auto messages = message_processor_.GetMessagesFromRawBytes(saved_bytes);
            net::post([self = shared_from_this(), messages = std::move(messages)]()
                {
                    (*self->message_handler_)(messages);
                });

            if (connection_->IsReconnectRequired()) {
                net::dispatch(connection_strand_, [self = shared_from_this()]() {
                    self->Reconnect();
                    });
            }
            else {
                Read();
            }
        }
        catch (const std::exception& e) {
            LOG_INFO("Catch exception in Client::OnRead");
            LOG_CRITICAL(e.what());
        }
    }

    void Client::Reconnect(bool secured) {
        net::io_context* ioc = nullptr;
        ioc = connection_->GetContext();

        if (secured) {
            ctx_.reset();
            ctx_ = connection::GetSSLContext();
            connection_ = std::make_shared<connection::Connection>(*ioc, *ctx_, write_strand_, read_strand_);
        }
        else {
            connection_ = std::make_shared<connection::Connection>(*ioc, write_strand_, read_strand_);
        }

        try {
            connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Reconnecting error: "s.append(e.what()));
            LOG_INFO("Retry after "s.append(std::to_string(reconnect_timeout_)).append(" sec"));
            reconnect_timer_.expires_after(std::chrono::seconds(reconnect_timeout_));
            reconnect_timer_.async_wait([self = shared_from_this()](const sys::error_code& ec) { 
                if (ec) {
                    logging::ReportError(ec, "Waiting reconnect timer");
                }
                self->message_handler_->UpdateConnection(self->connection_);
                self->message_processor_.FlushBuffer();
                self->Authorize();
                self->CapRequest();
                self->Join();
                self->Read();
                });
        }
    }

    std::string Client::GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names) {
        std::string command;
        bool is_first = true;
        for (const auto& channel_name : channels_names) {
            if (!is_first) {
                command += ",#";
            }
            command += channel_name;
            is_first = false;
        }
        return command;
    }

} // namespace irc