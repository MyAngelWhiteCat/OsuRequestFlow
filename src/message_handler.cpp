#include "message_handler.h"
#include "logging.h"


namespace irc {

    namespace handler {

        using namespace std::literals;
        using MessageType = irc::domain::MessageType;

        void MessageHandler::operator()(const std::vector<domain::Message>& messages) {
            std::ofstream fuckedup("FUCKED_UP.txt", std::ios::app);

            for (const auto& message : messages) {
                try {
                    switch (message.GetMessageType()) {
                    case MessageType::PING:
                        LOG_INFO("Get ping message!");
                        try {
                            SendPong(message.GetContent());
                            LOG_INFO("Send PONG"s.append(std::string(message.GetContent())));
                        } catch(...) {
                            LOG_CRITICAL("Sending pong error :(");
                        }
                        break;
                    case MessageType::PRIVMSG:
                        LOG_DEBUG(std::string(message.GetNick()).append(message.GetContent()));
                        // command_parser.parse(message.GetContent());
                        break;
                    case MessageType::UNKNOWN: // Debug only. In prod there is huge error if we get unknown message
                        fuckedup << message.GetContent() << std::endl;
                        LOG_ERROR("Unknow message type reseiced. Writed on log file");
                        break;
                    default:
                        std::ostringstream strm{};
                        domain::PrintMessageType(strm, message.GetMessageType());
                        LOG_INFO(strm.str().append(": ").append(std::string(message.GetContent())));
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

        void MessageHandler::SendPong(const std::string_view ball) {
            connection_->Write(std::string(domain::Command::PONG).append(std::string(ball)));
        }


    }

}