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
            EMPTY
        };

        struct Command {
            Command() = delete;
            static constexpr std::string_view NICK = "NICK "sv;
            static constexpr std::string_view PASS = "PASS oauth:"sv;
            static constexpr std::string_view CREQ = "CAP REQ :"sv;
            static constexpr std::string_view CRES = "CAP"sv;
            static constexpr std::string_view ACK = "CAP"sv;
            static constexpr std::string_view NAK = "CAP"sv;
            static constexpr std::string_view JOIN = "JOIN"sv;
            static constexpr std::string_view JOIN_CHANNEL = "JOIN #"sv;
            static constexpr std::string_view PART = "PART"sv;
            static constexpr std::string_view PART_CHANNEL = "PART #"sv;
            static constexpr std::string_view PONG = "PONG"sv;
            static constexpr std::string_view PING = "PING "sv;
            static constexpr std::string_view ROOMSTATE = "ROOMSTATE"sv;
            static constexpr std::string_view PRIVMSG = "PRIVMSG"sv;
            static constexpr std::string_view STATUSCODE = "STATUSCODE"sv;
        };

        struct Capabilityes {
            Capabilityes() = delete;
            static constexpr std::string_view COMMANDS = "twitch.tv/commands"sv;
            static constexpr std::string_view MEMBERSHIP = "twitch.tv/membership"sv;
            static constexpr std::string_view TAGS = "twitch.tv/tags"sv;
        };

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
            }
        }

        static std::string convert_utf8_to_ansi(const std::string& utf8_str) {
            int wide_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
            wchar_t* wide_str = new wchar_t[wide_len];
            MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wide_str, wide_len);

            int ansi_len = WideCharToMultiByte(CP_ACP, 0, wide_str, -1, nullptr, 0, nullptr, nullptr);
            char* ansi_str = new char[ansi_len];
            WideCharToMultiByte(CP_ACP, 0, wide_str, -1, ansi_str, ansi_len, nullptr, nullptr);

            std::string result(ansi_str);
            delete[] wide_str;
            delete[] ansi_str;
            return result;
        }
    } // namespace domain

} // namespace irc