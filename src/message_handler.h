#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "message.h"


namespace irc {

    namespace handler {

        using namespace std::literals;

        class MessageHandler {
        public:
            template <typename Owner>
            void operator()(const std::vector<domain::Message>& messages, std::shared_ptr<Owner> owner) {
                for (const auto& message : messages) {
                    if (message.GetMessageType() == irc::domain::MessageType::PRIVMSG) {
                        LOG_DEBUG(std::string(message.GetNick()).append(": "s).append(message.GetContent()));
                    }
                    else if (message.GetMessageType() == irc::domain::MessageType::UNKNOWN){
                        LOG_ERROR("Unknow message type reseiced: "s.append(message.GetContent()));
                    }
                }
                owner->Read();
            }

        };

    }

}