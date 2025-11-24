#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "message.h"

namespace commands {

    enum class CommandType {
        OsuRequest
    };

    struct Command {
    public:
        Command(CommandType type, std::string&& user_name, std::string&& content)
            : type_(type)
            , user_name_(std::move(user_name))
            , content_(std::move(content))
        {

        }

        CommandType type_;
        std::string user_name_;
        std::string content_;
        irc::domain::Role user_role_ = irc::domain::Role::MODERATOR; // dummy
    };

}