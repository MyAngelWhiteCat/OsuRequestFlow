#include "message.h"


namespace irc {

    namespace domain {

        domain::MessageType Message::GetMessageType() const {
            return message_type_;
        }

        std::string_view Message::GetContent() const {
            return content_;
        }

        std::string_view Message::GetNick() const {
            if (message_type_ != domain::MessageType::PRIVMSG) {
                throw std::logic_error("Only PRIMSG can have nick");
            }
            std::string badge = "display-name";
            auto it = badges_.find(badge);
            if (it != badges_.end()) {
                if (!it->second.empty()) {
                    return it->second[0];
                }
            }
            throw std::invalid_argument("Wrong username after parse badges");
            return "";
        }

        Badges Message::GetBadges() const {
            if (message_type_ != domain::MessageType::PRIVMSG) {
                throw std::logic_error("Only PRIMSG can have badges");
            }
            return badges_;
        }

        Message::Message(domain::MessageType message_type, std::string raw_part, std::string&& content, std::string&& badges)
            : message_type_(message_type)
            , raw_part_(raw_part)
            , content_(std::move(content))
        {
            std::string badge;
            bool badge_seted = false;
            std::string value;
            for (const char ch : badges) {
                if (!badge_seted && ch != '=') {
                    badge += ch;
                    continue;
                }
                else {
                    badge_seted = true;
                }
                if (ch == ',') {
                    badges_[badge].push_back(value);
                    value.clear();
                }
                else if (ch == ';') {
                    badges_[badge].push_back(value);
                    value.clear();
                    badge.clear();
                    badge_seted = false;
                }
                else if (ch != '=') {
                    value += ch;
                }
            }
        }

    }

}