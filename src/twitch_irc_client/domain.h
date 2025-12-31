#pragma once

#include <vector>
#include <string>
#include <string_view>

namespace irc {

    namespace domain {

        using namespace std::literals;

        struct IRC_EPS {
            static constexpr std::string_view HOST = "irc.chat.twitch.tv"sv;
            static constexpr std::string_view PORT = "6667"sv;
            static constexpr std::string_view SSL_PORT = "6697"sv;
        };

        enum class MessageType {
            ROOMSTATE,
            JOIN,
            PART,
            PRIVMSG,
            PING,
            STATUSCODE,
            CAPRES,
            UNKNOWN,
            EMPTY,
            CLEARCHAT,
            USERNOTICE
        };

        struct Command {
            Command() = delete;
            static constexpr std::string_view NICK = "NICK "sv;
            static constexpr std::string_view PASS = "PASS oauth:"sv;
            static constexpr std::string_view CREQ = "CAP REQ :"sv;
            static constexpr std::string_view CRES = "CAP"sv;
            static constexpr std::string_view ACK = "ACK"sv;
            static constexpr std::string_view NAK = "NAK"sv;
            static constexpr std::string_view JOIN = "JOIN"sv;
            static constexpr std::string_view JOIN_CHANNEL = "JOIN #"sv;
            static constexpr std::string_view PART = "PART"sv;
            static constexpr std::string_view PART_CHANNEL = "PART #"sv;
            static constexpr std::string_view PONG = "PONG"sv;
            static constexpr std::string_view PING = "PING"sv;
            static constexpr std::string_view ROOMSTATE = "ROOMSTATE"sv;
            static constexpr std::string_view PRIVMSG = "PRIVMSG"sv;
            static constexpr std::string_view STATUSCODE = "STATUSCODE"sv;
            static constexpr std::string_view CLEARCHAT = "CLEARCHAT"sv;
            static constexpr std::string_view USERNOTICE = "USERNOTICE"sv;
        };

        struct Capabilityes {
            Capabilityes() = delete;
            static constexpr std::string_view COMMANDS = "twitch.tv/commands"sv;
            static constexpr std::string_view MEMBERSHIP = "twitch.tv/membership"sv;
            static constexpr std::string_view TAGS = "twitch.tv/tags"sv;
        };

        static bool IsCRLF(const std::vector<char>& buff, size_t index) {
            if (index < buff.size() - 1) {
                return (buff[index] == '\r' && buff[index + 1] == '\n');
            }
            return false;
        }

        static std::vector<std::string_view> Split(std::string_view str) {
            std::vector<std::string_view> result;
            auto pos = str.find_first_not_of(" ");
            const auto pos_end = str.npos;
            while (pos != pos_end) {
                auto space = str.find(' ', pos);
                result.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
                pos = str.find_first_not_of(" ", space);
            }

            return result;
        }

        static bool IsNumber(const std::string_view str) {
            if (str.empty()) {
                return false;
            }

            for (int i = 0; i < str.size(); ++i) {
                if (!isdigit(str[i])) {
                    return false;
                }
            }

            return true;
        }

        template <typename Out>
        static void PrintMessageType(Out& out, const MessageType& type) {
            switch (type) {
            case MessageType::ROOMSTATE:
                out << Command::ROOMSTATE;
                break;
            case MessageType::JOIN:
                out << Command::JOIN;
                break;
            case MessageType::PART:
                out << Command::PART;
                break;
            case MessageType::PING:
                out << Command::PING;
                break;
            case MessageType::PRIVMSG:
                out << Command::PRIVMSG;
                break;
            case MessageType::STATUSCODE:
                out << Command::STATUSCODE;
                break;
            case MessageType::CAPRES:
                out << Command::CRES;
                break;
            case MessageType::CLEARCHAT:
                out << Command::CLEARCHAT;
                break;
            }

        }

    } // namespace domain

} // namespace irc