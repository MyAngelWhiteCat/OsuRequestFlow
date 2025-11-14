#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <string_view>

#include <iostream>
#include <memory>
#include <vector>

#include "command_parser.h"
#include "connection.h"
#include "message.h"


namespace irc {

    namespace handler {

        using namespace std::literals;

        namespace net = boost::asio;
        namespace sys = boost::system;
        using Strand = net::strand<net::io_context::executor_type>;

        class MessageHandler : public std::enable_shared_from_this<MessageHandler> {

        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, Strand& connection_strand)
                : connection_(connection)
                , connection_strand_(connection_strand)
            {

            }

            void operator()(const std::vector<domain::Message>& messages);

            void UpdateConnection(std::shared_ptr<connection::Connection>);

        private:
            const std::string RESET = "\033[0m";

            Strand& connection_strand_;
            std::shared_ptr<connection::Connection> connection_;
            commands::CommandParser command_parser_;

            void SendPong(const std::string_view content);
            std::string GetColorFromHex(const std::string& hexColor);
            void PrintTime(std::ostream& out);
            void ProcessCommand(const commands::Command& command);
        };

    }

}