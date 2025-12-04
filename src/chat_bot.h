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
            if (enable_chat_history_) {
                PrintableMessage msg;
                msg.name = message.GetNick();
                msg.colour = message.GetColorFromHex();
                msg.badge = irc::domain::RoleToString(message.GetRole());
                msg.message = message.GetContent().substr(1);
                last_messages_.push_back(std::move(msg));
            }
            if (auto command = parser_->Parse(std::move(message))) {
                executor_->Execute(std::move(*command));
            }

        }

        std::vector<PrintableMessage> GetLastMessages() {
            std::vector<PrintableMessage> tmp(last_messages_.begin(), last_messages_.end());
            last_messages_.clear();
            return tmp;
        }

        void ToggleChatHistory(bool toggle) {
            enable_chat_history_ = toggle;
            if (!toggle) {
                last_messages_.clear();
            }
        }

    private:
        std::shared_ptr<commands::CommandExecutor> executor_;
        std::shared_ptr<commands::CommandParser> parser_;
        std::vector<PrintableMessage> last_messages_;
        bool enable_chat_history_ = true;
    };

}