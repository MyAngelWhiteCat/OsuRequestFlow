#include "command.h"
#include "command_parser.h"
#include "command_executor.h"
#include "user_validator.h"


namespace chat_bot {

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