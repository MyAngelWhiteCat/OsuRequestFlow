#pragma once

#include <string>
#include <string_view>
#include <utility>

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
    };

}