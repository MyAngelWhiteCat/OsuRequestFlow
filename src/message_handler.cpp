#include "message_handler.h"
#include "logging.h"


namespace irc {

    namespace handler {

        using namespace std::literals;
        using MessageType = irc::domain::MessageType;

        void MessageHandler::operator()(const std::vector<domain::Message>& messages) {
            std::ofstream fuckedup("FUCKED_UP.txt", std::ios::app);
            std::osyncstream out(std::cout);

            try {
                for (const auto& message : messages) {
                    switch (message.GetMessageType()) {
                    case MessageType::PING:
                        LOG_INFO("Get ping message!");
                        try {
                            SendPong(message.GetContent());
                            LOG_INFO("Send PONG"s.append(std::string(message.GetContent())));
                        }
                        catch (...) {
                            LOG_CRITICAL("Sending pong error :(");
                        }
                        break;
                    case MessageType::PRIVMSG: // debug only
                        PrintTime(out);
                        out << GetColorFromHex(message.GetColorFromHex()) << message.GetNick() << RESET << message.GetContent() << "\n";
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

            }
            catch (const std::exception& e) {
                LOG_CRITICAL("Handling "s.append(e.what()));
            }
        }

        void MessageHandler::UpdateConnection(std::shared_ptr<connection::Connection> new_connection) {
            net::dispatch(connection_strand_, [self = shared_from_this(), new_connection]() {
                self->connection_ = new_connection;
                });
        }

        void MessageHandler::SendPong(const std::string_view ball) {
            net::dispatch(connection_strand_, [self = shared_from_this(), ball]() {
                self->connection_->Write(std::string(domain::Command::PONG).append(std::string(ball).append("\r\n")));
                });
        }

        std::string MessageHandler::GetColorFromHex(const std::string& hexColor) {
            const int too_dark = 80;
            if (hexColor.empty()) {
                return RESET;
            }
            int r = std::stoi(hexColor.substr(1, 2), nullptr, 16);
            int g = std::stoi(hexColor.substr(3, 2), nullptr, 16);
            int b = std::stoi(hexColor.substr(5, 2), nullptr, 16);

            if (r < too_dark && g < too_dark && b < too_dark) {
                r = g = b = too_dark + 1;
            }

            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }

        void MessageHandler::PrintTime(std::ostream& out) {
            auto now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(now);

            std::tm tm_buf;
            localtime_s(&tm_buf, &time);
            out << '[' << std::put_time(&tm_buf, "%H:%M:%S") << "] ";
        }


    }

}