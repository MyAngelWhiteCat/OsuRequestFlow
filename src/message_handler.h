#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "message.h"


namespace irc {

    namespace handler {

        class MessageHandler {
        public:
            template <typename Owner>
            void operator()(const std::vector<domain::Message>& messages, std::shared_ptr<Owner> owner) {
                for (const auto& message : messages) {
                    if (message.GetMessageType() == irc::domain::MessageType::PRIVMSG) {
                        std::cout << message.GetNick() << ": "
                            << message.GetContent() << std::endl;
                    }
                    else {
                        std::cout << message.GetContent() << std::endl;
                    }
                }
                owner->Read();
            }

        };

    }

}