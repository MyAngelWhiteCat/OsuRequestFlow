#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <memory>
#include <stdexcept>

#include "domain.h"
#include "message.h"
#include "auth_data.h"

namespace irc {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;

    using Strand = net::strand<net::io_context::executor_type>;

    void ReportError(sys::error_code ec, std::string_view where);

    template <typename MessageHandler>
    class Client : public std::enable_shared_from_this<Client<MessageHandler>> {
    public:
        Client() = delete;
        Client(net::io_context& ioc, ssl::context& ctx, Strand irc_strand)
            : socket_(ssl::stream<tcp::socket>(ioc, ctx))
            , irc_strand_(irc_strand)
        {
        }

        Client(net::io_context& ioc, Strand irc_strand)
            : socket_(tcp::socket(ioc))
            , irc_strand_(irc_strand)
            , message_handler_()
        {
        }

        ~Client() {
            if (ssl_connected_ || no_ssl_connected_) {
                for (const auto& [name, status] : channel_name_to_connect_status_) {
                    if (status) {
                        Part(name);
                    }
                }
                Disconnect();
            }
        }

        void Connect() {
            ec_.clear();

            ConnectionVisitor visitor(*this);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Connection");
            }
        }

        void Disconnect() {
            ec_.clear();

            DisconnectVisitor visitor(*this);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Disconnecting");
            }
        }

        void Join(const std::string_view channel_name) {
            ec_.clear();

            JoinVisitor visitor(*this, channel_name);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Join");
                channel_name_to_connect_status_[std::string(channel_name)] = false;
            }
            else {
                channel_name_to_connect_status_[std::string(channel_name)] = true;
            }
        }

        void Part(const std::string_view channel_name) {
            ec_.clear();

            PartVisitor visitor(*this, channel_name);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Part");
            }
            else {
                channel_name_to_connect_status_[std::string(channel_name)] = false;
            }
        }

        void Authorize(const domain::AuthorizeData& auth_data) {
            ec_.clear();

            AuthorizeVisitor visitor(*this, auth_data);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Authorize"s);
                authorized_ = false;
            }
        }

        void CapRequest() {
            ec_.clear();

            CapRequestVisitor visitor(*this);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "CapRequest");
            }
        }

        void Read() {
            ec_.clear();

            auto visitor = std::make_shared<ReadMessageVisitor>(this->shared_from_this());
            std::visit([&visitor](auto& arg) {
                (*visitor)(arg);
                }, socket_);
        }

        bool Connected() {
            CheckConnect();
            return ssl_connected_ || no_ssl_connected_;
        }

        void Pong(std::string_view ball) {
            ec_.clear();

            PingPongVisitor visitor(*this, ball);
            std::visit(visitor, socket_);
        }

        void CheckConnect() {
            ec_.clear();

            //TODO;
        }

        
    private:
        bool ssl_connected_ = false;
        bool no_ssl_connected_ = false;
        bool authorized_ = false;

        MessageHandler message_handler_;
        Strand irc_strand_;
        sys::error_code ec_;
        std::variant<tcp::socket, ssl::stream<tcp::socket>> socket_;
        std::unordered_map<std::string, bool> channel_name_to_connect_status_;

        domain::Message IdentifyMessageType(std::string_view raw_message) {
            const int EMPTY = 0;
            const int STATUSCODE_TAG_INDEX = 1;
            const int CAPABILITIES_REQUEST_TAG_INDEX = 1;
            const int PING_EXPECTED = 2;
            const int JOIN_PART_EXPECTED = 3;
            const int ROOMSTATE_OR_STATUSCODE_EXPECTED = 4;
            const int CORRECT_USER_MESSAGE_MINIMUM_SIZE = 4;
            const int CORRECT_STATUSCODE_MESSAGE_MINIMUM_SIZE = 3;

            auto split_raw_message = domain::Split(raw_message);

            switch (split_raw_message.size()) {
            case (EMPTY):
                return domain::Message(domain::MessageType::EMPTY, "");

            case (PING_EXPECTED):
                return CheckForPing(split_raw_message, raw_message);
                break;

            case (JOIN_PART_EXPECTED):
                return CheckForJoinPart(split_raw_message, raw_message);
                break;

            case (ROOMSTATE_OR_STATUSCODE_EXPECTED):
                return CheckForRoomstateOrStatusCode(split_raw_message, raw_message);

            default:
                if (split_raw_message.size() >= CORRECT_USER_MESSAGE_MINIMUM_SIZE) {
                    return CheckForUserMessage(split_raw_message, raw_message);
                }
                if (split_raw_message.size() >= CORRECT_STATUSCODE_MESSAGE_MINIMUM_SIZE) {
                    if (domain::IsNumber(split_raw_message[STATUSCODE_TAG_INDEX])) {
                        return domain::Message(domain::MessageType::STATUSCODE
                             , std::string(split_raw_message[STATUSCODE_TAG_INDEX]));
                    }
                }
                if (split_raw_message[CAPABILITIES_REQUEST_TAG_INDEX] == domain::Command::CRES) {
                    return domain::Message(domain::MessageType::CAPRES
                         , std::string(raw_message)); // Dummy
                }

                return domain::Message(domain::MessageType::UNKNOWN
                     , std::string(raw_message)); // Dummy
            }
            return domain::Message(domain::MessageType::UNKNOWN
                 , std::string(raw_message)); // Dummy
        }

        domain::Message CheckForPing(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message) {
            const int PING_COMMAND_INDEX = 0;
            const int PING_CONTENT_INDEX = 1;

            if (split_raw_message[PING_COMMAND_INDEX] == domain::Command::PING) {
                return domain::Message(domain::MessageType::PING
                     , std::string(split_raw_message[PING_CONTENT_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN
                 , std::string(raw_message));
        }

        domain::Message CheckForJoinPart(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message) {
            const int ACTION_TAG_INDEX = 1;
            const int CHANNEL_NAME_INDEX = 2;

            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::JOIN) {
                return domain::Message(domain::MessageType::JOIN
                     , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }
            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::PART) {
                return domain::Message(domain::MessageType::PART
                     , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN
                 , std::string(raw_message));
        }

        domain::Message CheckForRoomstateOrStatusCode(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message) {
            const int STATUSCODE_INDEX = 1;
            const int ROOMSTATE_CONTENT_INDEX = 1;
            const int ROOMSTATE_TAG_INDEX = 2;

            if (split_raw_message[ROOMSTATE_TAG_INDEX] == domain::Command::ROOMSTATE) {
                return domain::Message(domain::MessageType::ROOMSTATE
                     , std::string(split_raw_message[ROOMSTATE_CONTENT_INDEX]));
            }
            if (domain::IsNumber(split_raw_message[STATUSCODE_INDEX])) {
                return domain::Message(domain::MessageType::STATUSCODE
                     , std::string(split_raw_message[STATUSCODE_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN
                 , std::string(raw_message));
        }

        domain::Message CheckForUserMessage(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message) {
            const int BADGES_INDEX = 0;
            const int MSG_TAG_INDEX = 2;

            if (split_raw_message[MSG_TAG_INDEX] == domain::Command::PRIVMSG) {
                std::string user_content = GetUserMessageFromSplitRawMessage(split_raw_message);

                return std::move(domain::Message(domain::MessageType::PRIVMSG
                    , std::move(user_content)
                    , std::move(std::string(split_raw_message[BADGES_INDEX]))));
            }
            return std::move(domain::Message(domain::MessageType::EMPTY
                , std::move(std::string(raw_message))));
        }

        std::string GetUserMessageFromSplitRawMessage(const std::vector<std::string_view>& split_raw_message) {
            const int USER_MESSAGE_START = 4;

            std::string content;
            bool is_first = true;
            for (int i = USER_MESSAGE_START; i < split_raw_message.size(); ++i) {
                if (!is_first) {
                    content += ' ';
                }
                content += split_raw_message[i];
                is_first = false;
            }
            return content;
        }

        class ConnectionVisitor {
        public:
            explicit ConnectionVisitor(Client& client)
                : client_(client)
            {
            }

            void operator()(tcp::socket& socket) {
                tcp::resolver resolver(socket.get_executor()); // bad prac :(
                auto endpoints = resolver.resolve(domain::IRC_EPS::HOST, domain::IRC_EPS::PORT);
                net::connect(socket, endpoints, client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "Connection"sv);
                    return;
                }
                client_.no_ssl_connected_ = true;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                tcp::resolver resolver(socket.get_executor()); // again :(
                auto endpoints = resolver.resolve(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
                net::connect(socket.lowest_layer(), endpoints, client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "SSL Connection"sv);
                    return;
                }
                socket.lowest_layer().set_option(tcp::no_delay(true));
                socket.handshake(ssl::stream_base::client, client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "SSL Handshake"sv);
                    socket.lowest_layer().close();
                    return;
                }
                client_.ssl_connected_ = true;
            }

        private:
            Client& client_;
        };

        class PingPongVisitor {
        public:
            PingPongVisitor(Client& client, std::string_view ball)
                : client_(client)
                , ball_(ball)
            {
                if (ball.size() < domain::Command::PONG.size()) {
                    throw std::invalid_argument("incorrect PONG message"s);
                }
            }

            void operator()(tcp::socket& socket) {
                net::write(socket, net::buffer(std::string(domain::Command::PONG)
                    .append(std::string(ball_.substr(domain::Command::PING.size())))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL!! Sending PONG Error! Disconnection expected..."sv);
                }
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                net::write(socket, net::buffer(std::string(domain::Command::PONG)
                    .append(std::string(ball_.substr(domain::Command::PING.size())))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL!! Sending PONG Error! Disconnection expected..."sv);
                }
            }

        private:
            Client& client_;
            std::string_view ball_;
        };

        class ReadMessageVisitor : public std::enable_shared_from_this<ReadMessageVisitor> {
        public:
            explicit ReadMessageVisitor(std::shared_ptr<Client> client)
                : client_(client)
            {
            }

            void operator()(tcp::socket& socket) {
                ReadMessages(client_->no_ssl_connected_, socket);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                ReadMessages(client_->ssl_connected_, socket);
            }

        private:
            std::shared_ptr<Client> client_;
            net::streambuf streambuf_;

            template <typename Socket>
            void ReadMessages(bool is_connected, Socket& socket) {
                if (is_connected) {
                    net::async_read_until(socket, streambuf_, "\r\n"s, [self = this->shared_from_this()](const sys::error_code& ec, std::size_t bytes_readed) {
                        auto read_result = self->ExtractMessages(self->streambuf_);
                        self->OnRead(ec, read_result);
                        });
                }
                else {
                    throw std::runtime_error("Trying read socket without connection");
                }
            }

            std::vector<domain::Message> ExtractMessages(net::streambuf& streambuf) {
                std::vector<domain::Message> read_result;
                std::string line;
                std::istream is(&streambuf);
                while (std::getline(is, line)) {
                    domain::Message msg = client_->IdentifyMessageType(line);
                    if (msg.GetMessageType() == domain::MessageType::PING) {
                        client_->Pong(msg.GetContent());
                    }
                    read_result.push_back(msg);
                }

                return read_result;
            }

            void OnRead(const sys::error_code& ec, const std::vector<domain::Message>& read_result) {
                if (ec) {
                    ReportError(ec, "Reading");
                    return;
                }
                for (const auto& message : read_result) {
                    client_->message_handler_(message, client_->shared_from_this());
                }
            }
        };

        class DisconnectVisitor {
        public:
            explicit DisconnectVisitor(Client& client)
                : client_(client)
            {
            }

            void operator()(tcp::socket& socket) {
                socket.shutdown(net::socket_base::shutdown_send, ignor_);
                socket.close(client_.ec_);
                client_.no_ssl_connected_ = false;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                socket.shutdown(ignor_);
                socket.lowest_layer().close(client_.ec_);
                client_.ssl_connected_ = false;
            }

        private:
            Client& client_;
            sys::error_code ignor_;
        };

        class JoinVisitor {
        public:
            explicit JoinVisitor(Client& client, std::string_view channel_name)
                : client_(client)
                , channel_name_(channel_name)
            {
            }

            void operator()(tcp::socket& socket) {
                if (!client_.no_ssl_connected_) {
                    throw std::runtime_error("Join without connection attempt");
                }
                net::write(socket, net::buffer((std::string(domain::Command::JOIN_CHANNEL)
                    + std::string(channel_name_) + "\r\n"s)), client_.ec_);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                if (!client_.ssl_connected_) {
                    throw std::runtime_error("Join without connection attempt");
                }
                net::write(socket, net::buffer((std::string(domain::Command::JOIN_CHANNEL)
                    + std::string(channel_name_) + "\r\n"s)), client_.ec_);
            }

        private:
            Client& client_;
            std::string_view channel_name_;
        };

        class PartVisitor {
        public:
            explicit PartVisitor(Client& client, std::string_view channel_name)
                : client_(client)
                , channel_name_(channel_name)
            {
            }

            void operator()(tcp::socket& socket) {
                if (!client_.no_ssl_connected_) {
                    throw std::runtime_error("Part without connection attempt");
                }
                net::write(socket, net::buffer((std::string(domain::Command::PART_CHANNEL)
                    + std::string(channel_name_) + "\r\n"s)), client_.ec_);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                if (!client_.ssl_connected_) {
                    throw std::runtime_error("Part without connection attempt");
                }
                net::write(socket, net::buffer((std::string(domain::Command::PART_CHANNEL)
                    + std::string(channel_name_) + "\r\n"s)), client_.ec_);
            }

        private:
            Client& client_;
            std::string_view channel_name_;
        };


        class AuthorizeVisitor {
        public:
            explicit AuthorizeVisitor(Client& client, const domain::AuthorizeData& auth_data)
                : client_(client)
                , auth_data_(auth_data)
            {
            }

            void operator()(tcp::socket& socket) {
                if (!client_.no_ssl_connected_) {
                    throw std::runtime_error("Authorizing without connection attempt");
                }
                net::write(socket, net::buffer(auth_data_.GetAuthMessage()), client_.ec_);
                client_.authorized_ = true;
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                if (!client_.ssl_connected_) {
                    throw std::runtime_error("Authorizing without connection attempt");
                }
                net::write(socket, net::buffer(auth_data_.GetAuthMessage()), client_.ec_);
                client_.authorized_ = true;
            }

        private:
            Client& client_;
            domain::AuthorizeData auth_data_;
        };

        class CapRequestVisitor { // TODO: Optional capabilities. Check response
        public:
            explicit CapRequestVisitor(Client& client)
                : client_(client)
            {
            }

            void operator()(tcp::socket& socket) {
                if (!client_.no_ssl_connected_) {
                    throw std::runtime_error("Authorizing without connection attempt");
                }
                net::write(socket, net::buffer(std::string(domain::Command::CREQ)
                    + std::string(domain::Capabilityes::COMMANDS) + " "
                    + std::string(domain::Capabilityes::MEMBERSHIP) + " "
                    + std::string(domain::Capabilityes::TAGS) + "\r\n"), client_.ec_);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                if (!client_.ssl_connected_) {
                    throw std::runtime_error("Authorizing without connection attempt");
                }
                net::write(socket, net::buffer(std::string(domain::Command::CREQ)
                    + std::string(domain::Capabilityes::COMMANDS) + " "
                    + std::string(domain::Capabilityes::MEMBERSHIP) + " "
                    + std::string(domain::Capabilityes::TAGS) + "\r\n"), client_.ec_);
            }

        private:
            Client& client_;
        };

    };

} // namespace irc
