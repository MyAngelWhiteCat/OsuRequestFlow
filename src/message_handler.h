#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "message.h"
#include "connection.h"

#include <fstream>

namespace irc {

    namespace handler {

        using namespace std::literals;
        using Strand = net::strand<net::io_context::executor_type>;

        class MessageHandler {
        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, Strand& connection_strand)
                : connection_(connection)
                , connection_strand_(connection_strand)
            {
            }

            void operator()(const std::vector<domain::Message>& messages);

        private:
            Strand& connection_strand_;
            std::shared_ptr<connection::Connection> connection_;

            void SendPong(const std::string_view content);

        };

    }

}