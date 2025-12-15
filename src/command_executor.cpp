#include "command_executor.h"


#include "command.h"
#include "downloader.h"
#include "user_validator.h"
#include <utility>

namespace commands {

    void CommandExecutor::Execute(Command&& command) {
        if (!verificator_.Verify(command.user_name_, command.user_role_)) {
            std::cout << command.user_name_ << " Unverified" << std::endl;
            return;
        }
        switch (command.type_) {
        case CommandType::OsuRequest:
            downloader_->Download(command.content_);
        }
    }

    user_validator::UserVerificator* CommandExecutor::GetUserVerificator() {
        return &verificator_;
    }

    void CommandExecutor::SetUserVerificator(user_validator::UserVerificator&& verificator) {
        verificator_ = std::move(verificator);
    }

}