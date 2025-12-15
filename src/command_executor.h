#pragma once

#include "command.h"
#include "downloader.h"
#include "user_validator.h"
#include <utility>

namespace commands {

    class CommandExecutor {
    public:

        CommandExecutor(std::shared_ptr<downloader::Downloader> downloader)
            : downloader_(downloader)
        {

        }

        void Execute(Command&& command);
        user_validator::UserVerificator* GetUserVerificator();
        void SetUserVerificator(user_validator::UserVerificator&& verificator);

    private:
        std::shared_ptr<downloader::Downloader> downloader_;
        user_validator::UserVerificator verificator_;
    };

}

