#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>

#include "auth_data.h"
#include "connection.h"
#include "message_handler.h"
#include "message_processor.h"


namespace irc {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    template <typename ChatBot>
    class Client : public std::enable_shared_from_this<Client<ChatBot>> {
    public:
        Client() = delete;
        Client(net::io_context& ioc, std::shared_ptr<ChatBot> chat_bot, bool secured = true);

        void SetChatBot(std::shared_ptr<ChatBot> chat_bot);
        void Connect();
        void Disconnect();
        void Join(const std::vector<std::string_view>& channels_names);
        void Join(const std::string_view channel_name);
        void Join();
        void Part(const std::string_view channel_name);
        void Authorize();
        void Authorize(const domain::AuthorizeData& auth_data);
        void CapRequest();
        void Read();
        bool CheckConnect();
        void SetReconnectTimeout(int timeout_seconds);
        int GetReconnectTimeout();
        const std::unordered_set<std::string>& GetJoinedChannels();

    private:
        Strand write_strand_;
        Strand read_strand_;
        Strand connection_strand_;
        std::shared_ptr<ssl::context> ctx_;
        net::steady_timer reconnect_timer_;
        int reconnect_timeout_ = 30;

        message_processor::MessageProcessor message_processor_;
        std::shared_ptr<connection::Connection> connection_;
        std::shared_ptr<handler::MessageHandler<ChatBot>> message_handler_;

        bool authorized_ = false;
        std::unordered_set<std::string> joined_channels_;

        std::optional<std::string> join_command_buffer_;
        std::optional<std::string> auth_data_buffer_;

        void OnRead(std::vector<char>&& bytes);
        void Reconnect(bool secured = true);
        std::string GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names);
        void AddJoinCommandToBuffer(std::string_view join_command);
    };

    template <typename ChatBot>
    Client<ChatBot>::Client(net::io_context& ioc, std::shared_ptr<ChatBot> chat_bot, bool secured)
        : read_strand_(net::make_strand(ioc))
        , write_strand_(net::make_strand(ioc))
        , connection_strand_(net::make_strand(ioc))
        , reconnect_timer_(ioc)
        
    {
        if (secured) {
            ctx_ = connection::GetSSLContext();
            connection_ = std::make_shared<connection::Connection>(ioc, *ctx_, read_strand_, write_strand_);
            message_handler_ = std::make_shared<handler::MessageHandler<ChatBot>>(connection_, connection_strand_);
        }
        else {
            connection_ = std::make_shared<connection::Connection>(ioc, read_strand_, write_strand_);
            message_handler_ = std::make_shared<handler::MessageHandler<ChatBot>>(connection_, connection_strand_);
        }
        message_handler_->SetChatBot(chat_bot);
    }

    template<typename ChatBot>
    inline void Client<ChatBot>::SetChatBot(std::shared_ptr<ChatBot> chat_bot) {
        message_handler_->SetChatBot(chat_bot);
    }

    template <typename ChatBot>
    void Client<ChatBot>::Connect() {
        connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
    }

    template <typename ChatBot>
    void Client<ChatBot>::Disconnect() {
        connection_->Disconnect();
    }

    template <typename ChatBot>
    void Client<ChatBot>::Join(const std::vector<std::string_view>& channels_names) {
        std::string join_command = GetChannelNamesInStringCommand(channels_names);
        AddJoinCommandToBuffer(join_command);
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + join_command + "\r\n"s);
        for (const auto channel : channels_names) {
            joined_channels_.insert(std::string(channel));
        }
    }

    template <typename ChatBot>
    void Client<ChatBot>::Join(const std::string_view channel_name) {
        AddJoinCommandToBuffer(channel_name);
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + std::string(channel_name) + "\r\n"s);
        joined_channels_.insert(std::string(channel_name));
    }

    template <typename ChatBot>
    void Client<ChatBot>::Join() {
        if (!join_command_buffer_) {
            throw std::runtime_error("Empty reconnect buffer");
        }
        connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + *join_command_buffer_ + "\r\n"s);
    }

    template <typename ChatBot>
    void Client<ChatBot>::Part(const std::string_view channel_name) {
        connection_->Write(std::string(domain::Command::PART_CHANNEL) + std::string(channel_name) + "\r\n"s);
        joined_channels_.erase(std::string(channel_name));
    }

    template <typename ChatBot>
    void Client<ChatBot>::Authorize() {
        if (!auth_data_buffer_) {
            throw std::runtime_error("Empty reconnect buffer");
        }
        connection_->Write(*auth_data_buffer_);
    }

    template <typename ChatBot>
    void Client<ChatBot>::Authorize(const domain::AuthorizeData& auth_data) {
        auth_data_buffer_ = auth_data.GetAuthMessage();
        connection_->Write(*auth_data_buffer_);
    }

    template <typename ChatBot>
    void Client<ChatBot>::CapRequest() {
        connection_->Write(std::string(domain::Command::CREQ)
            + std::string(domain::Capabilityes::COMMANDS) + " "
            + std::string(domain::Capabilityes::MEMBERSHIP) + " "
            + std::string(domain::Capabilityes::TAGS) + "\r\n");
    }

    template <typename ChatBot>
    void Client<ChatBot>::Read() {
        auto process_message = net::bind_executor(read_strand_, [self = this->shared_from_this()](std::vector<char>&& bytes) {
            self->OnRead(std::move(bytes));
            });
        connection_->AsyncRead(process_message);
    }

    template <typename ChatBot>
    bool Client<ChatBot>::CheckConnect() {
        return connection_->IsConnected();
    }

    template <typename ChatBot>
    void Client<ChatBot>::SetReconnectTimeout(int timeout) {
        reconnect_timeout_ = timeout;
    }

    template<typename ChatBot>
    inline int Client<ChatBot>::GetReconnectTimeout() {
        return reconnect_timeout_;
    }

    template<typename ChatBot>
    inline const std::unordered_set<std::string>& Client<ChatBot>::GetJoinedChannels() {
        return joined_channels_;
    }

    template <typename ChatBot>
    void Client<ChatBot>::OnRead(std::vector<char>&& bytes) {
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

    template <typename ChatBot>
    void Client<ChatBot>::Reconnect(bool secured) {
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

    template <typename ChatBot>
    std::string Client<ChatBot>::GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names) {
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

    template<typename ChatBot>
    inline void Client<ChatBot>::AddJoinCommandToBuffer(std::string_view join_command) {
        if (join_command_buffer_) {
            (*join_command_buffer_) += ",#"s.append(std::string(join_command));
        }
        else {
            join_command_buffer_ = std::string(join_command);
        }
    }

} // namespace irc
