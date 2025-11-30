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
        Command(CommandType type, std::string&& user_name, std::string&& content, irc::domain::Role&& role)
            : type_(type)
            , user_name_(std::move(user_name))
            , content_(std::move(content))
            , user_role_(std::move(role))
        {

        }

        CommandType type_;
        std::string user_name_;
        std::string content_;
        irc::domain::Role user_role_;
    };

}