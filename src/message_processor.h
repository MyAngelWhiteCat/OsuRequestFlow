#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "domain.h"
#include "message.h"
#include "auth_data.h"

namespace irc {

    namespace net = boost::asio;
    namespace sys = boost::system;
    namespace ssl = net::ssl;
    using net::ip::tcp;
    using namespace std::literals;


    class MessageProcessor {
    public:
        std::vector<domain::Message> ProcessMessage(std::vector<char>& streambuf) {
            return ExtractMessages(streambuf);
        }

    private:
        std::string last_read_incomplete_message_;

        std::vector<domain::Message> ExtractMessages(std::vector<char>& raw_read_result) {
            std::vector<domain::Message> read_result;
            std::string raw_message;
            if (!last_read_incomplete_message_.empty()) {
                raw_message = last_read_incomplete_message_;
                last_read_incomplete_message_.clear();
            }

            for (int i = 0; i < raw_read_result.size() - 1; ++i) {
                if (!IsCRLF(raw_read_result[i], raw_read_result[i + 1])) {
                    raw_message += raw_read_result[i];
                }
                else {
                    read_result.push_back(IdentifyMessageType(raw_message));
                    raw_message.clear();
                    ++i;
                }
            }
            if (!raw_message.empty()) {
                last_read_incomplete_message_ = raw_message;
            }

            return read_result;
        }

        domain::Message IdentifyMessageType(std::string_view raw_message) {
            const size_t EMPTY = 0;
            const size_t STATUSCODE_TAG_INDEX = 1;
            const size_t CAPABILITIES_REQUEST_TAG_INDEX = 1;
            const size_t PING_EXPECTED = 2;
            const size_t CLEARCHAT_TAG_INDEX = 2;
            const size_t JOIN_PART_OR_CLEARCHAT_EXPECTED = 3;
            const size_t CORRECT_STATUSCODE_MESSAGE_MINIMUM_SIZE = 3;
            const size_t ROOMSTATE_OR_STATUSCODE_EXPECTED = 4;
            const size_t CORRECT_USER_MESSAGE_MINIMUM_SIZE = 4;

            auto split_raw_message = domain::Split(raw_message);

            switch (split_raw_message.size()) {
            case (EMPTY):
                return domain::Message(domain::MessageType::EMPTY, "");

            case (PING_EXPECTED):
                return CheckForPing(split_raw_message, raw_message);
                break;

            case (JOIN_PART_OR_CLEARCHAT_EXPECTED):
                return CheckForJoinPartOrClearChat(split_raw_message, raw_message);
                break;

            case (ROOMSTATE_OR_STATUSCODE_EXPECTED):
                return CheckForRoomstateOrStatusCode(split_raw_message, raw_message);

            default:
                if (split_raw_message.size() >= CORRECT_USER_MESSAGE_MINIMUM_SIZE) {
                    return CheckForUserMessage(split_raw_message, raw_message);
                }
                if (split_raw_message.size() >= CORRECT_STATUSCODE_MESSAGE_MINIMUM_SIZE) {
                    if (domain::IsNumber(split_raw_message[STATUSCODE_TAG_INDEX])) {
                        return domain::Message(domain::MessageType::STATUSCODE
                            , std::string(split_raw_message[STATUSCODE_TAG_INDEX]));
                    }
                }
                if (split_raw_message[CAPABILITIES_REQUEST_TAG_INDEX] == domain::Command::CRES) {
                    return domain::Message(domain::MessageType::CAPRES
                        , std::string(raw_message)); // Dummy
                }

                return domain::Message(domain::MessageType::UNKNOWN
                    , std::string(raw_message)); // Dummy
            }
            return domain::Message(domain::MessageType::UNKNOWN
                , std::string(raw_message)); // Dummy
        }

        domain::Message CheckForPing(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int PING_COMMAND_INDEX = 0;
            const int PING_CONTENT_INDEX = 1;

            if (split_raw_message[PING_COMMAND_INDEX] == domain::Command::PING) {
                return domain::Message(domain::MessageType::PING
                    , std::string(split_raw_message[PING_CONTENT_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN
                , std::string(raw_message));
        }

        domain::Message CheckForJoinPartOrClearChat(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int ACTION_TAG_INDEX = 1;
            const int CHANNEL_NAME_INDEX = 2;
            const int CLEARCHAT_TAG_INDEX = 2;

            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::JOIN) {
                return domain::Message(domain::MessageType::JOIN
                    , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }
            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::PART) {
                return domain::Message(domain::MessageType::PART
                    , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }
            if (split_raw_message[CLEARCHAT_TAG_INDEX] == domain::Command::CLEARCHAT) {
                return domain::Message(domain::MessageType::CLEARCHAT, std::string(raw_message)); // TODO?
            }
            return domain::Message(domain::MessageType::UNKNOWN, std::string(raw_message));
        }

        domain::Message CheckForRoomstateOrStatusCode(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int STATUSCODE_INDEX = 1;
            const int ROOMSTATE_CONTENT_INDEX = 1;
            const int ROOMSTATE_TAG_INDEX = 2;

            if (split_raw_message[ROOMSTATE_TAG_INDEX] == domain::Command::ROOMSTATE) {
                return domain::Message(domain::MessageType::ROOMSTATE
                    , std::string(split_raw_message[ROOMSTATE_CONTENT_INDEX]));
            }
            if (domain::IsNumber(split_raw_message[STATUSCODE_INDEX])) {
                return domain::Message(domain::MessageType::STATUSCODE
                    , std::string(split_raw_message[STATUSCODE_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN, std::string(raw_message));
        }

        domain::Message CheckForUserMessage(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int BADGES_INDEX = 0;
            const int MSG_TAG_INDEX = 2;

            if (split_raw_message[MSG_TAG_INDEX] == domain::Command::PRIVMSG) {
                std::string user_content = GetUserMessageFromSplitRawMessage(split_raw_message);

                return domain::Message(domain::MessageType::PRIVMSG
                    , std::string(raw_message)
                    , std::move(user_content)
                    , std::string(split_raw_message[BADGES_INDEX]));
            }
            return domain::Message(domain::MessageType::UNKNOWN, std::string(raw_message));
        }

        std::string GetUserMessageFromSplitRawMessage(const std::vector<std::string_view>& split_raw_message) {
            const int USER_MESSAGE_START = 4;

            std::string content;
            bool is_first = true;
            for (int i = USER_MESSAGE_START; i < split_raw_message.size(); ++i) {
                if (!is_first) {
                    content += ' ';
                }
                content += split_raw_message[i];
                is_first = false;
            }
            return content;
        }

        bool IsCRLF(char lhs, char rhs) {
            return (lhs == '\r' && rhs == '\n');
        }
    };

}