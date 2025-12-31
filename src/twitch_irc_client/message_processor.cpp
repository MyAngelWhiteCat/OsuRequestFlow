#include "message_processor.h"
#include "message.h"
#include "domain.h"

#include <vector>
#include <iostream>
#include <exception>
#include <string>
#include <string_view>
#include <utility>
#include <optional>


namespace irc {

    namespace message_processor {

        void MessageProcessor::FlushBuffer() {
            if (!last_read_incomplete_message_.empty()) {
                last_read_incomplete_message_.clear();
            }
        }

        std::vector<domain::Message> MessageProcessor::GetMessagesFromRawBytes(const std::vector<char>& raw_bytes) {
            std::vector<domain::Message> read_result;
            std::string raw_message;

            try {
                if (!last_read_incomplete_message_.empty()) {
                    raw_message = last_read_incomplete_message_;
                    last_read_incomplete_message_.clear();
                }

                for (int i = 0; i < raw_bytes.size(); ++i) {
                    if (domain::IsCRLF(raw_bytes, i)) {
                        read_result.push_back(IdentifyMessageType(raw_message));
                        raw_message.clear();
                        ++i;
                        continue;
                    }
                    raw_message += raw_bytes[i];
                }
                if (!raw_message.empty()) {
                    last_read_incomplete_message_ = raw_message;
                }
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
            }

            return read_result;
        }

        domain::Message MessageProcessor::IdentifyMessageType(std::string_view raw_message) {
            try {
                auto split_raw_message = domain::Split(raw_message);
                domain::Message message(domain::MessageType::UNKNOWN, std::string(raw_message));

                // 99.99% of all messages. No any unnecessary operations required!
                if (split_raw_message.size() >= USER_MESSAGE_MINIMUM_SIZE) {
                    if (auto msg = CheckForUserMessage(split_raw_message, raw_message)) {
                        return *msg;
                    }
                }

                switch (split_raw_message.size()) {
                case (EMPTY):
                    return domain::Message(domain::MessageType::EMPTY, "");

                case (JOIN_PART_EXPECTED):
                    return CheckForJoinPart(split_raw_message, raw_message); // TODO: split to 2 methods

                default:
                    if (split_raw_message.size() >= PING_MESSAGE_MINIMUM_SIZE) {
                        if (auto msg = CheckForPing(split_raw_message, raw_message)) {
                            return *msg;
                        }
                    }
                    if (split_raw_message.size() >= ROOMSTATE_MINIMUM_SIZE) {
                        if (auto msg = CheckForRoomstate(split_raw_message)) {
                            return message.TakeTypeAndMegre(std::move(*msg));
                        }
                    }

                    if (split_raw_message.size() >= STATUSCODE_MINIMUM_SIZE) {
                        if (auto msg = CheckForStatusCode(split_raw_message)) {
                            return message.TakeTypeAndMegre(std::move(*msg));
                        }
                    }

                    if (split_raw_message.size() >= CAPRES_MINIMUM_SIZE) {
                        if (auto msg = CheckForCapRes(split_raw_message)) {
                            return message.TakeTypeAndMegre(std::move(*msg));
                        }
                        else if (auto msg = CheckForClearChat(split_raw_message)) {
                            return message.TakeTypeAndMegre(std::move(*msg));
                        }
                    }
                }
                return message;
            }
            catch (const std::exception& e) {
                LOG_CRITICAL(e.what());
            }
        }

        std::optional<domain::Message> MessageProcessor::CheckForCapRes(const std::vector<std::string_view>& split_raw_message) {
            return CheckForCapRes(split_raw_message, "");
        }

        std::optional<domain::Message> MessageProcessor::CheckForCapRes(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const size_t CAPABILITIES_REQUEST_TAG_INDEX = 1;
            const size_t CAPABILITIES_MESSAGE_MINIMUM_SIZE = 4;

            if (split_raw_message[CAPABILITIES_REQUEST_TAG_INDEX] == domain::Command::CRES) {
                return domain::Message(domain::MessageType::CAPRES
                    , std::string(raw_message)); // Dummy
            }
            return std::nullopt;
        }

        std::optional<domain::Message> MessageProcessor::CheckForPing(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_content) {
            const int PING_COMMAND_INDEX = 0;
            const int PING_CONTENT_INDEX = 1;
            const int PING_COMMAND_SIZE = 4;

            if (split_raw_message[PING_COMMAND_INDEX] == domain::Command::PING) {
                return domain::Message(domain::MessageType::PING
                    , std::string(raw_content.substr(PING_COMMAND_SIZE)));
            }
            return std::nullopt;
        }

        std::optional<domain::Message> MessageProcessor::CheckForClearChat(const std::vector<std::string_view>& split_raw_message) {
            return CheckForClearChat(split_raw_message, "");
        }

        std::optional<domain::Message> MessageProcessor::CheckForClearChat(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int CLEARCHAT_TAG_INDEX = 2;
            if (split_raw_message[CLEARCHAT_TAG_INDEX] == domain::Command::CLEARCHAT) {
                return domain::Message(domain::MessageType::CLEARCHAT, std::string(raw_message)); // TODO?
            }
            return std::nullopt;
        }

        domain::Message MessageProcessor::CheckForJoinPart(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int ACTION_TAG_INDEX = 1;
            const int CHANNEL_NAME_INDEX = 2;

            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::JOIN) {
                return domain::Message(domain::MessageType::JOIN
                    , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }
            if (split_raw_message[ACTION_TAG_INDEX] == domain::Command::PART) {
                return domain::Message(domain::MessageType::PART
                    , std::string(split_raw_message[CHANNEL_NAME_INDEX]));
            }

            return domain::Message(domain::MessageType::UNKNOWN, std::string(raw_message));
        }

        std::optional<domain::Message> MessageProcessor::CheckForStatusCode(const std::vector<std::string_view>& split_raw_message) {
            return CheckForStatusCode(split_raw_message, "");
        }

        std::optional<domain::Message> MessageProcessor::CheckForStatusCode(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const size_t CORRECT_STATUSCODE_MESSAGE_MINIMUM_SIZE = 3;
            const int STATUSCODE_INDEX = 1;

            if (domain::IsNumber(split_raw_message[STATUSCODE_INDEX])) {
                return domain::Message(domain::MessageType::STATUSCODE
                    , std::string(split_raw_message[STATUSCODE_INDEX]));
            }
            return std::nullopt;
        }

        std::optional<domain::Message> MessageProcessor::CheckForRoomstate(const std::vector<std::string_view>& split_raw_message) {
            return CheckForRoomstate(split_raw_message, "");
        }
        std::optional<domain::Message> MessageProcessor::CheckForRoomstate(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int ROOMSTATE_CONTENT_INDEX = 1;
            const int ROOMSTATE_TAG_INDEX = 2;

            if (split_raw_message.size() > ROOMSTATE_TAG_INDEX && split_raw_message[ROOMSTATE_TAG_INDEX] == domain::Command::ROOMSTATE) {
                return domain::Message(domain::MessageType::ROOMSTATE
                    , std::string(split_raw_message[ROOMSTATE_CONTENT_INDEX]));
            }

            return std::nullopt;
        }

        std::optional<domain::Message> MessageProcessor::CheckForUserMessage(const std::vector<std::string_view>& split_raw_message
            , std::string_view raw_message) {
            const int BADGES_INDEX = 0;
            const int MSG_TAG_INDEX = 2;

            if (split_raw_message[MSG_TAG_INDEX] == domain::Command::PRIVMSG
                || split_raw_message[MSG_TAG_INDEX] == domain::Command::USERNOTICE) {
                std::string user_content = GetUserMessageFromSplitRawMessage(split_raw_message);

                if (split_raw_message[MSG_TAG_INDEX] == domain::Command::PRIVMSG) {
                    return domain::Message(domain::MessageType::PRIVMSG
                        , std::move(user_content)
                        , std::string(split_raw_message[BADGES_INDEX]));
                }
                else {
                    return domain::Message(domain::MessageType::USERNOTICE // TODO: process usernotice
                        , std::move(user_content)
                        , std::string(split_raw_message[BADGES_INDEX]));
                }
            }

            return std::nullopt;
        }

        std::string MessageProcessor::GetUserMessageFromSplitRawMessage(const std::vector<std::string_view>& split_raw_message) {
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
            if (content[0] == ':') {
                return content.substr(1);
            }
            return content;
        }


    } // namesapce message_processor

} // namespace irc