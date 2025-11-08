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
#include "message_handler.h"
#include "connection.h"

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
        Client(net::io_context& ioc, ssl::context& ctx)
            : read_strand_(net::make_strand(ioc))
            , write_strand_(net::make_strand(ioc))
            , connection_(std::make_shared<connection::Connection>(ioc, ctx, write_strand_, read_strand_))

        {
        }

        Client(net::io_context& ioc)
            : read_strand_(net::make_strand(ioc))
            , write_strand_(net::make_strand(ioc))
            , connection_(std::make_shared<connection::Connection>(ioc, write_strand_, read_strand_))
        {

        }

        void Connect() {
            connection_->Connect(domain::IRC_EPS::HOST, domain::IRC_EPS::SSL_PORT);
        }

        void Disconnect() {
            connection_->Disconnect();
        }

        void Join(const std::vector<std::string_view>& channels_names) {
            std::string command = GetChannelNamesInStringCommand(channels_names);
            connection_->Write(std::string(domain::Command::JOIN_CHANNEL) + command + "\r\n"s);
        }

        void Part(const std::vector<std::string_view>& channels_names) {
            std::string command = GetChannelNamesInStringCommand(channels_names);
            connection_->Write(std::string(domain::Command::PART_CHANNEL) + command + "\r\n"s);
        }


        void Authorize(const domain::AuthorizeData& auth_data) {
            connection_->Write(auth_data.GetAuthMessage());
        }

        void CapRequest() {
            connection_->Write(std::string(domain::Command::CREQ)
                + std::string(domain::Capabilityes::COMMANDS) + " "
                + std::string(domain::Capabilityes::MEMBERSHIP) + " "
                + std::string(domain::Capabilityes::TAGS) + "\r\n");
        }

        void Read() {
            auto process_message = net::bind_executor(read_strand_, [self = this->shared_from_this()](std::vector<char>&& bytes) {
                self->OnRead(std::move(bytes));
                });
            connection_->Read(process_message);
        }

        //TODO
        bool CheckConnect() {
            return connection_->IsConnected();
        }

    private:
        Strand write_strand_;
        Strand read_strand_;

        handler::MessageHandler message_handler_;
        message_processor::MessageProcessor message_processor_;
        std::shared_ptr<connection::Connection> connection_;

        bool authorized_ = false;
        std::unordered_map<std::string, bool> channel_name_to_connect_status_;

        void OnRead(std::vector<char>&& bytes) {
            std::vector<char> saved_bytes = std::move(bytes);
            auto messages = message_processor_.GetMessagesFromRawBytes(saved_bytes);
            net::post([self = shared_from_this(), messages = std::move(messages)]()
                {
                    self->message_handler_(messages);
                });
            Read();
        }

        std::string GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names) {
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
    };

} // namespace irc
