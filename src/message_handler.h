#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "message.h"

#include <fstream>

namespace irc {

    namespace handler {

        using namespace std::literals;

        class MessageHandler {
        public:
            void operator()(const std::vector<domain::Message>& messages) {
                for (const auto& message : messages) {
                    try {
                        if (message.GetMessageType() == irc::domain::MessageType::PRIVMSG) {
                            LOG_DEBUG(std::string(message.GetNick()).append(": "s).append(message.GetContent()));
                            //std::osyncstream(std::cout) << "Message from " << message.GetNick() << " handled in thread: " << std::this_thread::get_id() << "\n";
                        }
                        else if (message.GetMessageType() == irc::domain::MessageType::UNKNOWN) {
                            std::ofstream fuckedup("FUCKED_UP.txt", std::ios::app);
                            fuckedup << message.GetContent() << std::endl;
                            LOG_ERROR("Unknow message type reseiced. Writed on log file");
                        }
                    }
                    catch (const std::exception& e) {
                        std::ofstream fuckedup("FUCKED_UP.txt", std::ios::app);
                        fuckedup << "[Thats fucked up] " << "\nReason: " << e.what()
                            << "\n|" << message.GetRawPart() << "\n[End of Fucked up]" << std::endl;
                        LOG_FUCKUP("FUCK. But its logged");
                    }
                }
            }

        };

    }

}