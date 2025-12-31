#pragma once

#include <string>
#include <utility>
#include <memory>

#include "twitch_irc_client/message.h"
#include "chat_bot/command_executor.h"
#include "chat_bot/user_validator.h"

namespace commands {

    class Command {
    public:
        Command() = default;

        Command(std::unique_ptr<BaseCommandExecutor>&& executor)
            : executor_(std::move(executor))
        {

        }

        void Execute(std::string_view user_name, irc::domain::Role user_role);

        void AddContent(std::string&& content);
        void AddContent(std::string_view content);

        void SetMinimumUserRole(irc::domain::Role role);

        void SetWhiteListOnly(bool status);
        void AddUserInWhiteList(std::string_view user_name);
        void AddUsersInWhiteList(const std::vector<std::string>& users_names);
        void RemoveUserFromWhiteList(std::string_view user_name);
        void AddUserInBlackList(std::string_view user_name);
        void AddUsersInBlackList(const std::vector<std::string>& users_names);
        void RemoveUserFromBlackList(std::string_view user_name);
        void SetRoleLevel(int level);
        int GetRoleLevel();
        bool GetWhiteListOnly() const;
        std::unordered_set<std::string>* GetWhiteList();
        std::unordered_set<std::string>* GetBlackList();

    private:
        std::unique_ptr<BaseCommandExecutor> executor_{nullptr};
        user_validator::UserVerificator verificator_;

        irc::domain::Role minimum_user_role_{3};
        std::string content_;
    };

}