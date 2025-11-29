#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <syncstream>

#include "auth_data.h"
#include "domain.h"
#include "logging.h"
#include "message.h"


namespace irc {

    using namespace std::literals;

    namespace message_processor {

        const size_t EMPTY = 0;
        const size_t STATUSCODE_TAG_INDEX = 1;
        const size_t CAPABILITIES_REQUEST_TAG_INDEX = 1;
        const size_t PING_MESSAGE_MINIMUM_SIZE = 2;
        const size_t CLEARCHAT_TAG_INDEX = 2;
        const size_t ROOMSTATE_MINIMUM_SIZE = 2;
        const size_t JOIN_PART_EXPECTED = 3;
        const size_t STATUSCODE_MINIMUM_SIZE = 3;
        const size_t USER_MESSAGE_MINIMUM_SIZE = 4;
        const size_t CLEARCHAT_MINIMUM_SIZE = 4;
        const size_t CAPRES_MINIMUM_SIZE = 4;

        class MessageProcessor {
        public:
            std::vector<domain::Message> GetMessagesFromRawBytes(const std::vector<char>& raw_bytes);
            void FlushBuffer();

        private:
            std::string last_read_incomplete_message_;

            domain::Message IdentifyMessageType(std::string_view raw_message);
            std::optional<domain::Message> CheckForCapRes(const std::vector<std::string_view>& split_raw_message);
            std::optional<domain::Message> CheckForCapRes(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            std::optional<domain::Message> CheckForPing(const std::vector<std::string_view>& split_raw_message, std::string_view raw_content);
            std::optional<domain::Message> CheckForClearChat(const std::vector<std::string_view>& split_raw_message);
            std::optional<domain::Message> CheckForClearChat(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            domain::Message CheckForJoinPart(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            std::optional<domain::Message> CheckForStatusCode(const std::vector<std::string_view>& split_raw_message);
            std::optional<domain::Message> CheckForStatusCode(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            std::optional<domain::Message> CheckForRoomstate(const std::vector<std::string_view>& split_raw_message);
            std::optional<domain::Message> CheckForRoomstate(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            std::optional<domain::Message> CheckForUserMessage(const std::vector<std::string_view>& split_raw_message, std::string_view raw_message);
            std::string GetUserMessageFromSplitRawMessage(const std::vector<std::string_view>& split_raw_message);
        };

    } // namesapce message_processor

} // namespace irc