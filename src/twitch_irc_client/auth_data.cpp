#include "auth_data.h"
#include "domain.h"


namespace irc {

    namespace domain {

        std::string AuthorizeData::GetAuthMessage() const {
            std::string data;
            data.append(Command::PASS).append(token_);
            data.append("\r\n");
            data.append(Command::NICK).append(nick_);
            data.append("\r\n");
            return data;
        }

        void AuthorizeData::SetNick(std::string_view nick) {
            nick_ = std::string(nick);
        }

        void AuthorizeData::SetToken(std::string_view token) {
            token_ = std::string(token);
        }


    }

}