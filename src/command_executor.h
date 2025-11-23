#pragma once

#include "command.h"
#include "downloader.h"
#include "user_validator.h"

namespace commands {

    class CommandExecutor {
    public:

        CommandExecutor(std::shared_ptr<downloader::Downloader> downloader)
            : downloader_(downloader)
        {

        }

        void Execute(Command&& command) {
            switch (command.type_) {
            case CommandType::OsuRequest:
                
            }
        }


    private:
        std::shared_ptr<downloader::Downloader> downloader_;
        

    };

}

