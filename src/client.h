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
        Client(net::io_context& ioc, ssl::context& ctx);
        Client(net::io_context& ioc);

        void Connect();
        void Disconnect();
        void Join(const std::vector<std::string_view>& channels_names);
        void Part(const std::vector<std::string_view>& channels_names);
        void Authorize(const domain::AuthorizeData& auth_data);
        void CapRequest();
        void Read();
        bool CheckConnect();

    private:
        Strand write_strand_;
        Strand read_strand_;

        handler::MessageHandler message_handler_;
        message_processor::MessageProcessor message_processor_;
        std::shared_ptr<connection::Connection> connection_;

        bool authorized_ = false;
        std::unordered_map<std::string, bool> channel_name_to_connect_status_;

        void OnRead(std::vector<char>&& bytes);
        std::string GetChannelNamesInStringCommand(std::vector<std::string_view> channels_names);
    };

} // namespace irc
