#pragma once

#include "chat_bot/user_validator.h"

#include <string_view>


namespace commands {

    class BaseCommandExecutor {
    public:
        virtual void operator()([[maybe_unused]] std::string_view content) = 0;
    
    };

}

