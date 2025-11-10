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

        class MessageHandler {
        public:
            MessageHandler(std::shared_ptr<connection::Connection> connection, std::mutex& connection_mutex)
                : connection_(connection)
                , connection_mutex_(connection_mutex)
            {
            }

            void operator()(const std::vector<domain::Message>& messages);

        private:
            std::mutex& connection_mutex_;
            std::shared_ptr<connection::Connection> connection_;

            void SendPong(const std::string_view content);

        };

    }

}