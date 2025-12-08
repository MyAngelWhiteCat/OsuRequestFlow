#pragma once 

#include "command.h"
#include "command_parser.h"
#include "command_executor.h"


namespace chat_bot {

    struct PrintableMessage {
        std::string name;
        std::string badge;
        std::string colour;
        std::string message;
    };

    class ChatBot {
    public:
        ChatBot(std::shared_ptr<commands::CommandExecutor> executor
            , std::shared_ptr<commands::CommandParser> parser)
            : executor_(executor)
            , parser_(parser)
        {

        }
         
        void ParseAndExecute(irc::domain::Message&& message) {
            if (auto command = parser_->Parse(std::move(message))) {
                executor_->Execute(std::move(*command));
            }
        }

    private:
        std::shared_ptr<commands::CommandExecutor> executor_;
        std::shared_ptr<commands::CommandParser> parser_;
    };

}