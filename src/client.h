#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <memory>
#include <stdexcept>
#include <optional>

#include "domain.h"
#include "message.h"
#include "logging.h"
#include "auth_data.h"
#include "message_processor.h"

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
                std::vector<std::string_view> joined;
                joined.reserve(channel_name_to_connect_status_.size());
                for (const auto& [name, status] : channel_name_to_connect_status_) {
                    if (status) {
                        joined.push_back(name);
                    }
                }
                Part(joined);
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

        void Join(const std::vector<std::string_view>& channels_names) {
            ec_.clear();

            JoinVisitor visitor(*this, channels_names);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Join");
                for (const auto& channel_name : channels_names) {
                    channel_name_to_connect_status_[std::string(channel_name)] = false; // Naaaah..This is not how it should work..
                }
            }
            else {
                for (const auto& channel_name : channels_names) {
                    channel_name_to_connect_status_[std::string(channel_name)] = true;
                }
            }
        }

        void Part(const std::vector<std::string_view>& channels_names) {
            ec_.clear();

            PartVisitor visitor(*this, channels_names);
            std::visit(visitor, socket_);
            if (ec_) {
                ReportError(ec_, "Part");
            }
            else {
                for (const auto& name : channels_names) {
                    channel_name_to_connect_status_[std::string(name)] = false;
                }
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

        MessageProcessor message_processor_;

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
                    .append(std::string(ball_.substr(domain::Command::PING.size())).append("\r\n"))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL!! Sending PONG Error! Disconnect expected..."sv);
                }
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                net::write(socket, net::buffer(std::string(domain::Command::PONG)
                    .append(std::string(ball_.substr(domain::Command::PING.size())).append("\r\n"))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL!! Sending PONG Error! Disconnect expected..."sv);
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
            std::vector<domain::Message> read_result_;
            std::shared_ptr<std::vector<char>> buffer_ = std::make_shared<std::vector<char>>(512);

            template <typename Socket>
            void ReadMessages(bool is_connected, Socket& socket) {
                if (is_connected) {
                    net::async_read(socket, net::buffer(*buffer_), [self = this->shared_from_this()]
                    (const sys::error_code& ec, std::size_t bytes_readed) mutable {
                        self->read_result_ = self->client_->message_processor_.ProcessMessage(*self->buffer_);
                        self->OnRead(ec);
                        });
                }
                else {
                    throw std::runtime_error("Trying read socket without connection");
                }
            }


            void OnRead(const sys::error_code& ec) {
                if (ec) {
                    ReportError(ec, "Reading");
                    return;
                }
                client_->message_handler_(read_result_, client_->shared_from_this());
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
            explicit JoinVisitor(Client& client, const std::vector<std::string_view>& channels_names)
                : client_(client)
                , channels_names_(channels_names)
            {
            }

            void operator()(tcp::socket& socket) {
                JoinChannel(socket, client_.no_ssl_connected_);

            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                JoinChannel(socket, client_.ssl_connected_);
            }

        private:
            Client& client_;
            std::vector<std::string_view> channels_names_;

            template <typename Socket>
            void JoinChannel(Socket& socket, bool connected) {
                if (!connected) {
                    throw std::runtime_error("Join without connection attempt");
                }
                std::string command;
                bool is_first = true;
                for (const auto& name : channels_names_) {
                    if (!is_first) {
                        command += ",#";
                    }
                    command += std::string(name);
                    is_first = false;
                }
                net::write(socket, net::buffer((std::string(domain::Command::JOIN_CHANNEL)
                    + command + "\r\n"s)), client_.ec_);
            }
        };

        class PartVisitor {
        public:
            explicit PartVisitor(Client& client, const std::vector<std::string_view>& channels_names)
                : client_(client)
                , channels_names_(channels_names)
            {
            }

            void operator()(tcp::socket& socket) {
                PartChannels(socket, client_.no_ssl_connected_);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                PartChannels(socket, client_.ssl_connected_);
            }

        private:
            Client& client_;
            std::vector<std::string_view> channels_names_;

            template <typename Socket>
            void PartChannels(Socket& socket, bool connected) {
                if (!connected) {
                    throw std::runtime_error("Part without connection attempt");
                }
                std::string command;
                bool is_first = true;
                for (const auto& channel_name : channels_names_) {
                    if (!is_first) {
                        command += ",#";
                    }
                    command += channel_name;
                    is_first = false;
                }
                net::write(socket, net::buffer((std::string(domain::Command::PART_CHANNEL)
                    + command + "\r\n"s)), client_.ec_);
            }
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
