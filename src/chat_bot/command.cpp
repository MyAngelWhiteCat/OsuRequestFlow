#pragma once

#include <string>
#include <memory>

#include "chat_bot/user_validator.h"
#include "chat_bot/command.h"
#include <twitch_irc_client/message.h>
#include <vector>

namespace commands {

    void Command::Execute(std::string_view user_name, irc::domain::Role user_role) {
        if (verificator_.Verify(user_name, user_role)) {
            (*executor_)(content_);
        }
    }

    void Command::AddContent(std::string&& content) {
        content_ = std::move(content);
    }

    void Command::AddContent(std::string_view content) {
        content_ = std::string(content);
    }

    void Command::SetMinimumUserRole(irc::domain::Role role) {
        minimum_user_role_ = role;
    }

    void Command::SetWhiteListOnly(bool status) {
        verificator_.SetWhiteListOnly(status);
    }

    void Command::AddUserInWhiteList(std::string_view user_name) {
        verificator_.AddUserInWhiteList(user_name);
    }

    void Command::AddUsersInWhiteList(const std::vector<std::string>& users_names) {
        for (const auto& user_name : users_names) {
            verificator_.AddUserInWhiteList(user_name);
        }
    }

    void Command::RemoveUserFromWhiteList(std::string_view user_name) {
        verificator_.RemoveUserFromWhiteList(user_name);
    }

    void Command::AddUserInBlackList(std::string_view user_name) {
        verificator_.AddUserInBlackList(user_name);
    }

    void Command::AddUsersInBlackList(const std::vector<std::string>& users_names) {
        for (const auto& user_name : users_names) {
            verificator_.AddUserInBlackList(user_name);
        }
    }

    void Command::RemoveUserFromBlackList(std::string_view user_name) {
        verificator_.RemoveUserFromBlackList(user_name);
    }

    void Command::SetRoleLevel(int level) {
        verificator_.SetRoleLevel(level);
    }

    int Command::GetRoleLevel() {
        return verificator_.GetRoleLevel();
    }

    bool Command::GetWhiteListOnly() const {
        return verificator_.GetWhiteListOnly();
    }

    std::unordered_set<std::string>* Command::GetWhiteList() {
        return verificator_.GetWhiteList();
    }

    std::unordered_set<std::string>* Command::GetBlackList() {
        return verificator_.GetBlackList();
    }

}