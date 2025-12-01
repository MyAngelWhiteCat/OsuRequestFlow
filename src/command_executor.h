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

        void Execute(Command&& command) {
            if (!verificator_.Verify(command.user_name_, command.user_role_)) {
                std::cout << command.user_name_ << " Unverified" << std::endl;
                return;
            }
            switch (command.type_) {
            case CommandType::OsuRequest:
                downloader_->Download(command.content_);
            }
        }

        user_validator::UserVerificator* GetUserVerificator() {
            return &verificator_;
        }

        void SetUserVerificator(user_validator::UserVerificator&& verificator) {
            verificator_ = std::move(verificator);
        }

    private:
        std::shared_ptr<downloader::Downloader> downloader_;
        user_validator::UserVerificator verificator_;
    };

}

