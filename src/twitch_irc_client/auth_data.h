#pragma once

#include <string>
#include <string_view>

namespace irc {

    namespace domain {

        using namespace std::literals;

        struct AuthorizeData {
            AuthorizeData() = default;
            AuthorizeData(std::string_view nick, std::string_view token)
                : nick_(std::string(nick)), token_(std::string(token)) {
            }

            std::string GetAuthMessage() const;
            void SetNick(std::string_view nick);
            void SetToken(std::string_view token);

        private:
            std::string nick_ = "justinfan12345"s;
            std::string token_ = "undefined"s;
        };

    }

}