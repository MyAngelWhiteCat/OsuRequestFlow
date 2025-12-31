#include "irc_client.h"

namespace irc {

    Client::Client(net::io_context& ioc, std::shared_ptr<chat_bot::ChatBot> chat_bot, bool secured)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_strand_(net::make_strand(ioc))
        , reconnect_timer_(ioc)

    {
        if (secured) {
            ctx_ = connection::GetSSLContext();
            connection_ = std::make_shared<connection::Connection>(ioc, *ctx_, read_strand_, write_strand_);
            message_handler_ = std::make_shared<handler::MessageHandler>(connection_, connection_strand_);
        }
        else {
            connection_ = std::make_shared<connection::Connection>(ioc, read_strand_, write_strand_);
            message_handler_ = std::make_shared<handler::MessageHandler>(connection_, connection_strand_);
        }
        message_handler_->SetChatBot(chat_bot);
    }

    void Client::SetChatBot(std::shared_ptr<chat_bot::ChatBot> chat_bot) {
        message_handler_->SetChatBot(chat_bot);
    }

    void Client::Connect() {
        connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
    }

    void Client::Disconnect() {
        connection_->Disconnect();
    }

    void Client::Join(const std::vector<std::string_view>& channels_names) {
        std::string join_command = GetChannelNamesInStringCommand(channels_names);
        AddJoinCommandToBuffer(join_command);
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + join_command + "\r\n"s);
        for (const auto channel : channels_names) {
            joined_channels_.insert(std::string(channel));
        }
    }

    void Client::Join(const std::string_view channel_name) {
        AddJoinCommandToBuffer(channel_name);
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + std::string(channel_name) + "\r\n"s);
        joined_channels_.insert(std::string(channel_name));
    }

    void Client::Join() {
        if (!join_command_buffer_) {
            throw std::runtime_error("Empty reconnect buffer");
        }
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + *join_command_buffer_ + "\r\n"s);
    }

    void Client::Part(const std::string_view channel_name) {
        connection_->Write(std::string(domain::Command::PART_CHANNEL) + std::string(channel_name) + "\r\n"s);
        joined_channels_.erase(std::string(channel_name));
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
        auto process_message = net::bind_executor(read_strand_, [self = this->shared_from_this()](std::vector<char>&& bytes) {
            self->OnRead(std::move(bytes));
            });
        connection_->AsyncRead(process_message);
    }

    bool Client::CheckConnect() {
        return connection_->IsConnected();
    }

    void Client::SetReconnectTimeout(int timeout) {
        reconnect_timeout_ = timeout;
    }

    int Client::GetReconnectTimeout() const {
        return reconnect_timeout_;
    }

    const std::unordered_set<std::string>& Client::GetJoinedChannels() {
        return joined_channels_;
    }

    void Client::OnRead(std::vector<char>&& bytes) {
        try {
            std::vector<char> saved_bytes = std::move(bytes);
            auto messages = message_processor_.GetMessagesFromRawBytes(saved_bytes);
            net::post([self = this->shared_from_this(), messages = std::move(messages)]() mutable
                {
                    (*self->message_handler_)(std::move(messages));
                });

            if (connection_->IsReconnectRequired()) {
                net::dispatch(connection_strand_, [self = this->shared_from_this()]() {
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
            reconnect_timer_.expires_after(std::chrono::seconds(reconnect_timeout_));
            reconnect_timer_.async_wait([self = this->shared_from_this(), secured](const sys::error_code& ec) {
                if (ec) {
                    logging::ReportError(ec, "Waiting reconnect timer");
                }
                std::string_view port = secured ? domain::IRC_EPS::SSL_PORT : domain::IRC_EPS::PORT;
                self->connection_->Connect(domain::IRC_EPS::HOST, port);
                self->message_handler_->UpdateConnection(self->connection_);
                self->message_processor_.FlushBuffer();
                self->Authorize();
                self->Read();
                self->CapRequest();
                self->Join();
                });
        }
        catch (const std::exception& e) {
            LOG_ERROR("Reconnecting error: "s.append(e.what()));
            LOG_INFO("Retry after "s.append(std::to_string(reconnect_timeout_)).append(" sec"));
            Reconnect(secured);
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

    inline void Client::AddJoinCommandToBuffer(std::string_view join_command) {
        if (join_command_buffer_) {
            (*join_command_buffer_) += ",#"s.append(std::string(join_command));
        }
        else {
            join_command_buffer_ = std::string(join_command);
        }
    }

}