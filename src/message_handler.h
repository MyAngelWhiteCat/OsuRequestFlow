#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <string>
#include <string_view>

#include <iostream>
#include <memory>
#include <vector>

#include "chat_bot.h"
#include "connection.h"
#include "message.h"


namespace irc {

    namespace handler {

        using namespace std::literals;

        namespace net = boost::asio;
        namespace sys = boost::system;
        using Strand = net::strand<net::io_context::executor_type>;
        using MessageType = irc::domain::MessageType;

        template <typename ChatBot>
        class MessageHandler : public std::enable_shared_from_this<MessageHandler<ChatBot>> {

        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, Strand& connection_strand)
                : connection_(connection)
                , connection_strand_(connection_strand)
            {

            }

            void operator()(std::vector<domain::Message>&& messages);

            void UpdateConnection(std::shared_ptr<connection::Connection> new_connection);
            void SetChatBot(std::shared_ptr<ChatBot> chat_bot);

        private:
            const std::string RESET = "\033[0m";

            Strand& connection_strand_;
            std::shared_ptr<connection::Connection> connection_;
            std::shared_ptr<ChatBot> chat_bot_{ nullptr };


            void SendPong(const std::string_view ball);
            std::string GetColorFromHex(const std::string& hexColor);
            void PrintTime(std::ostream& out);
        };

        template <typename ChatBot>
        void MessageHandler<ChatBot>::operator()(std::vector<domain::Message>&& messages) {
            std::ofstream fuckedup("FUCKED_UP.txt", std::ios::app);
            std::osyncstream out(std::cout);

            try {
                for (auto& message : messages) {
                    switch (message.GetMessageType()) {
                    case MessageType::PING:
                        SendPong(message.GetContent());
                        break;
                    case MessageType::PRIVMSG: // debug only
                        logging::output_mutex.lock();
                        //PrintTime(out);
                        out/* << GetColorFromHex(message.GetColorFromHex()) */<< message.GetNick() /*<< RESET*/ << message.GetContent() << "\n";
                        logging::output_mutex.unlock();
                        if (!chat_bot_) {
                            return;
                        }
                        net::post([self = this->shared_from_this(), message = std::move(message)]() mutable
                            {
                                self->chat_bot_->ParseAndExecute(std::move(message));
                            });
                        break;
                    case MessageType::UNKNOWN: // Debug only. In prod there is huge error if we get unknown message
                        fuckedup << message.GetContent() << std::endl;
                        LOG_ERROR("Unknow message type reseiced. Writed on log file");
                        break;
                    //default:
                        //std::ostringstream strm{};
                        //domain::PrintMessageType(strm, message.GetMessageType());
                        //LOG_INFO(strm.str().append(": ").append(std::string(message.GetContent())));
                    }
                }

            }
            catch (const std::exception& e) {
                LOG_CRITICAL("Handling "s.append(e.what()));
            }
        }

        template <typename ChatBot>
        void MessageHandler<ChatBot>::UpdateConnection(std::shared_ptr<connection::Connection> new_connection) {
            net::dispatch(connection_strand_, [self = this->shared_from_this(), new_connection]() {
                self->connection_ = new_connection;
                });
        }

        template <typename ChatBot>
        void MessageHandler<ChatBot>::SetChatBot(std::shared_ptr<ChatBot> chat_bot) {
            chat_bot_ = chat_bot;
        }

        template <typename ChatBot>
        void MessageHandler<ChatBot>::SendPong(const std::string_view ball) {
            net::dispatch(connection_strand_, [self = this->shared_from_this(), ball]() {
                self->connection_->AsyncWrite(std::string(domain::Command::PONG).append(std::string(ball).append("\r\n")));
                });
        }

        template <typename ChatBot>
        std::string MessageHandler<ChatBot>::GetColorFromHex(const std::string& hexColor) {
            const int too_dark = 80;
            if (hexColor.empty()) {
                return RESET;
            }
            int r = std::stoi(hexColor.substr(1, 2), nullptr, 16);
            int g = std::stoi(hexColor.substr(3, 2), nullptr, 16);
            int b = std::stoi(hexColor.substr(5, 2), nullptr, 16);

            if (r < too_dark && g < too_dark && b < too_dark) {
                r = g = b = too_dark + 1;
            }

            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }

        template <typename ChatBot>
        void MessageHandler<ChatBot>::PrintTime(std::ostream& out) {
            auto now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(now);

            std::tm tm_buf;
            localtime_s(&tm_buf, &time);
            out << '[' << std::put_time(&tm_buf, "%H:%M:%S") << "] ";
        }

    }


}
