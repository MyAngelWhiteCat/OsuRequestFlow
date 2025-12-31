#include "chat_bot/chat_bot.h"
#include "logger/logging.h"

#include <utility>

namespace chat_bot {

    void ChatBot::SetCommandStart(char ch) {
        command_start_ = ch;
    }

    char ChatBot::GetCommandStart() const {
        return command_start_;
    }

    void ChatBot::AddCommand(std::string_view command_name, commands::Command&& command) {
        name_to_command_[std::string(command_name)] = std::move(command);
    }

    void ChatBot::AddMode(std::string_view mode_name, Mode&& mode) {
        name_to_mode_[std::string(mode_name)] = std::move(mode);
    }

    commands::Command* ChatBot::GetCommand(std::string_view command_name) {
        if (auto it = name_to_command_.find(std::string(command_name));
            it != name_to_command_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    Mode* ChatBot::GetMode(std::string_view mode_name) {
        if (auto it = name_to_command_.find(std::string(mode_name));
            it != name_to_command_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // case 1 - user:!command 
    // case 2 - user:!command some text for command execution
    void ChatBot::ParseAndExecute(irc::domain::Message&& message) {
        auto line = message.GetContent();
        if (line.empty()) {
            return;
        }

        net::post(ioc_, [self = shared_from_this(), message]() mutable {
            self->UseModes(std::move(message)); });
        net::post(ioc_, [self = shared_from_this(), message = std::move(message)]() mutable {
            self->ProcessCommand(std::move(message)); });
    }

    void ChatBot::UseModes(irc::domain::Message&& msg) {
        try {
            for (auto& [_, mode] : name_to_mode_) {
                mode.AddContent(msg.GetContent());
                mode.Execute(msg.GetNick(), msg.GetRole());
            }
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }
    }

    void ChatBot::ProcessCommand(irc::domain::Message&& msg) {
        try {
            auto line = msg.GetContent();
            if (line[0] == command_start_) {
                std::string command;
                std::string content;
                size_t command_end = line.find_first_of(' ');
                if (command_end == std::string::npos) {
                    command = std::string(line.substr(1));
                }
                else {
                    command = std::string(line.substr(1, command_end - 1));
                    content = std::string(line.substr(command_end + 1));
                }
                if (auto it = name_to_command_.find(command); it != name_to_command_.end()) {
                    it->second.AddContent(std::move(content));
                    it->second.Execute(msg.GetNick(), msg.GetRole());
                }
                else {
                    LOG_ERROR("Unknown command");
                }
            }
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }
    }
}
