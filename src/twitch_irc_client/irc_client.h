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

#include "twitch_irc_client/auth_data.h"
#include "connection/connection.h"
#include "chat_bot/chat_bot.h"
#include "twitch_irc_client/message_handler.h"
#include "twitch_irc_client/message_processor.h"


namespace irc {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    class Client : public std::enable_shared_from_this<Client> {
    public:
        Client() = delete;
        Client(net::io_context& ioc, std::shared_ptr<chat_bot::ChatBot> chat_bot, bool secured = true);

        void SetChatBot(std::shared_ptr<chat_bot::ChatBot> chat_bot);
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
        int GetReconnectTimeout() const;
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
        std::shared_ptr<handler::MessageHandler> message_handler_;

        bool authorized_ = false;
        std::unordered_set<std::string> joined_channels_;

        std::optional<std::string> join_command_buffer_;
        std::optional<std::string> auth_data_buffer_;

        void OnRead(std::vector<char>&& bytes);
        void Reconnect(bool secured = true);
        std::string GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names);
        void AddJoinCommandToBuffer(std::string_view join_command);
    };


} // namespace irc
