#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <variant>
#include <stdexcept>

#include "domain.h"

namespace irc {

    using Badges = std::unordered_map<std::string, std::vector<std::string>>;

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = boost::asio::ssl;
    using net::ip::tcp;
    using namespace std::literals;


    void ReportError(sys::error_code ec, std::string_view where);

    class Message {
    public:
        Message() = delete;
        Message(domain::MessageType message_type, std::string&& content)
            : content_(content)
            , message_type_(message_type)
        {

        }

        Message(domain::MessageType message_type, std::string&& content, std::string&& badges);

        domain::MessageType GetMessageType() const;
        std::string_view GetContent() const;
        std::string_view GetNick() const;
        Badges GetBadges() const;
    private:
        domain::MessageType message_type_;
        std::string content_;
        Badges badges_;
    };

    struct AuthorizeData {
        AuthorizeData() = default;
        AuthorizeData(std::string_view nick, std::string_view token)
            : nick_(std::string(nick)), token_(std::string(token)) {}
        
        std::string GetAuthMessage() const;
        void SetNick(std::string_view nick);
        void SetToken(std::string_view token);

    private:
        std::string nick_ = "justinfan12345"s;
        std::string token_ = "1234567890abcdef1234567890abcdef"s;
    };

    class Client {
    public:
        Client() = delete;
        Client(net::io_context& ioc);
        Client(net::io_context& ioc, ssl::context& ctx 
            /*set verify mode and verify path
        EXAMPLE:
            net::io_context ioc;
            ssl::context ctx {ssl::context::tlsv12client};
            ctx_.set_verify_mode(ssl::verify_peer);
            GetVerifySerts();
            ctx_.set_default_verify_paths();
            irc::Client(ioc, ctx);
        */);

        void SSL_Connect();
        void NOSSL_Connect();
        void Disconnect();
        void Join(const std::string_view chanel_name);
        void Authorize(const AuthorizeData& auth_data);
        void CapRequest();
        std::vector<Message> Read();
        bool Connected();

    private:
        bool ssl_connected_ = false;
        bool no_ssl_connected_ = false;
        bool authorized_ = false;
        sys::error_code ec_;
        std::variant<tcp::socket, ssl::stream<tcp::socket>> socket_;

        void Connect(bool secured = true);
        void Pong(std::string_view ball);
        void CheckConnect();
        Message IdentifyMessageType(std::string_view raw_message);

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
                ReportError(client_.ec_, "Connection"sv);
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                tcp::resolver resolver(socket.get_executor()); // again :(
                auto endpoints = resolver.resolve(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
                net::connect(socket.lowest_layer(), endpoints, client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "SSL Connection"sv);
                    return;
                }
                socket.handshake(ssl::stream_base::client, client_.ec_);
                ReportError(client_.ec_, "SSL Handshake"sv);
            }

        private:
            Client& client_;
        };

        class PingPongVisitor {
        public:
            explicit PingPongVisitor(Client& client, std::string_view ball)
                : client_(client)
                , ball_(ball)
            {
                if (ball.size() < domain::Command::PONG.size()) {
                    throw std::invalid_argument("incorrect PONG message"s);
                }
            }

            void operator()(tcp::socket& socket) {
                net::write(socket, net::buffer(std::string(domain::Command::PONG)
                    .append(std::string(ball_.substr(domain::Command::PONG.size())))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL. Sending PONG Error! Disconnection expected..."sv);
                }
            }

            void operator()(ssl::stream<tcp::socket>& socket) {
                net::write(socket, net::buffer(std::string(domain::Command::PONG)
                    .append(std::string(ball_.substr(domain::Command::PONG.size())))), client_.ec_);
                if (client_.ec_) {
                    ReportError(client_.ec_, "CRITICAL. Sending PONG Error! Disconnection expected..."sv);
                }
            }

        private:
            Client& client_;
            std::string_view ball_;
        };
    };

} // namespace irc
