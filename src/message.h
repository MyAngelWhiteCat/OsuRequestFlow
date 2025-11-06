#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <string_view>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <stdexcept>

#include "domain.h"

namespace irc {

    namespace domain {

        using Badges = std::unordered_map<std::string, std::vector<std::string>>;

        class Message {
        public:
            Message() = delete;
            Message(MessageType message_type, std::string&& content)
                : content_(content)
                , message_type_(message_type)
            {

            }

            Message(domain::MessageType message_type, std::string raw_part, std::string&& content, std::string&& badges);
            bool operator==(const Message& other) const;
            Message TakeTypeAndMegre(Message&& other);
            MessageType GetMessageType() const;
            std::string_view GetContent() const;
            std::string_view GetNick() const;
            Badges GetBadges() const;
            std::string GetRawPart() const {
                return raw_part_;
            }
        private:
            std::string raw_part_; // for debug only;
            MessageType message_type_;
            std::string content_;
            Badges badges_;
        };

    }

}