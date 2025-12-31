#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include <string>
#include <string_view>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include "chat_bot/chat_bot.h"
#include "connection/connection.h"
#include "twitch_irc_client/message.h"

namespace irc {

    namespace handler {

        using namespace std::literals;

        namespace net = boost::asio;
        namespace sys = boost::system;
        using Strand = net::strand<net::io_context::executor_type>;
        using MessageType = irc::domain::MessageType;

        class MessageHandler : public std::enable_shared_from_this<MessageHandler> {

        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, Strand& connection_strand)
                : connection_(connection)
                , connection_strand_(connection_strand)
            {

            }

            void operator()(std::vector<domain::Message>&& messages);

            void UpdateConnection(std::shared_ptr<connection::Connection> new_connection);
            void SetChatBot(std::shared_ptr<chat_bot::ChatBot> chat_bot);

        private:
            Strand& connection_strand_;
            std::shared_ptr<connection::Connection> connection_;
            std::shared_ptr<chat_bot::ChatBot> chat_bot_{ nullptr };

            void SendPong(const std::string_view ball);
        };



    }

}
