#include "client.h"

namespace irc {

    Client::Client(net::io_context& ioc, ssl::context& ctx)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_(std::make_shared<connection::Connection>(ioc, ctx, write_strand_, read_strand_))
        , fake_activity_timer_(ioc)
    {
    }

    Client::Client(net::io_context& ioc)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_(std::make_shared<connection::Connection>(ioc, write_strand_, read_strand_))
        , fake_activity_timer_(ioc)
    {

    }

    void Client::Connect() {
        connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
        //RunFakeActivity();
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
        auto process_message = net::bind_executor(read_strand_, [self = this->shared_from_this()](std::vector<char>&& bytes) {
            self->OnRead(std::move(bytes));
            });
        connection_->Read(process_message);
    }

    //TODO
    bool Client::CheckConnect() {
        return connection_->IsConnected();
    }

    void Client::RunFakeActivity() {
        fake_activity_timer_.expires_after(std::chrono::minutes(5));
        fake_activity_timer_.async_wait([self = shared_from_this()](const sys::error_code& ec) {
            if (ec) {
                LOG_ERROR("Error wile faking activity. Disconnect expected after 10 min");
            }
            else {
                if (self->CheckConnect()) {
                    self->connection_->Write("TIME");
                    self->RunFakeActivity();
                }
                else {
                    LOG_ERROR("Faking activity without connection");
                }
            }
            });
    }

    void Client::OnRead(std::vector<char>&& bytes) {
        std::vector<char> saved_bytes = std::move(bytes);
        auto messages = message_processor_.GetMessagesFromRawBytes(saved_bytes);
        net::post([self = shared_from_this(), messages = std::move(messages)]()
            {
                self->message_handler_(messages);
            });
        Read();

        if (connection_->IsReconnected()) {
            Authorize();
            Join();
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