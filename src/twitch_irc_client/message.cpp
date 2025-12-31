#include "message.h"
#include <memory>


namespace irc {

    namespace domain {

        Message::Message(domain::MessageType message_type, std::string&& content, std::string&& badges)
            : message_type_(message_type)
            , content_(std::move(content))
        {
            if (badges.empty()) return;
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

            SetRole();

        }

        Message Message::TakeTypeAndMegre(Message&& other) {
            if (other.message_type_ == MessageType::PRIVMSG) {
                for (auto& [badge, value] : other.badges_) {
                    for (auto& v : value) {
                        this->badges_[badge].push_back(std::move(v));
                    };
                }
            }

            this->message_type_ = other.message_type_;
            content_.append(other.content_);
            return *this;
        }

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

        Role Message::GetRole() const {
            if (message_type_ != domain::MessageType::PRIVMSG) {
                throw std::logic_error("Only PRIMSG can have badges");
            }
            return role_;
        }

        std::string Message::GetColorFromHex() const {
            auto it = badges_.find("color");
            if (it != badges_.end() && !it->second.empty() && !it->second[0].empty()) {
                return it->second[0].substr(1);
            }
            return "";
        }

        void Message::SetRole() {
            if (auto it = badges_.find("badges"); it != badges_.end()) {
                if (it->second.empty()) {
                    role_ = Role::EMPTY;
                    return;
                }
                auto& raw_badges = it->second;
                for (const auto& raw_badge : raw_badges) {
                    size_t delimer = raw_badge.find('/');
                    if (delimer == std::string::npos) { 
                        continue; 
                    }
                    
                    std::string_view badge(raw_badge.data(), delimer);
                    std::string_view value(raw_badge.data() + delimer + 1, raw_badge.size() - delimer - 1);
                    
                    if (value == "0") {
                        continue;
                    }
                    if (badge == "broadcaster" && role_ < Role::BROADCASTER) {
                        role_ = Role::BROADCASTER;
                        return;
                    }
                    else if (badge == "moderator" && role_ < Role::MODERATOR) {
                        role_ = Role::MODERATOR;
                        return;
                    }
                    else if (badge == "vip" && role_ < Role::VIP) {
                        role_ = Role::VIP;
                        return;
                    }
                    else if (badge == "subscriber" && role_ < Role::SUBSCRIBER) {
                        role_ = Role::SUBSCRIBER;
                        return;
                    }
                }
            }
            
        }

        bool Message::operator==(const Message& other) const {
            return this->badges_ == other.badges_
                && this->content_ == other.content_;
        }

    }

}