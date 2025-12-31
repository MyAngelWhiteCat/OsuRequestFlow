#pragma once

#include "twitch_irc_client/message.h" // !! Role only required!!!

#include <string>
#include <string_view>
#include <memory>
#include <unordered_set>
#include <vector>


namespace commands {

    namespace user_validator {

        using namespace std::literals;

        class RoleFilter {
        public:
            bool CheckRole(irc::domain::Role role) {
                return role >= accept_from_;
            }

            void SetLevel(int level) {
                accept_from_ = static_cast<irc::domain::Role>(level);
            }

            int GetLevel() const {
                return static_cast<int>(accept_from_);
            }

        private:
            irc::domain::Role accept_from_ = irc::domain::Role::MODERATOR;
        };

        class UserVerificator {
        public:
            UserVerificator()
                : black_list_(std::make_unique<std::unordered_set<std::string>>())
                , white_list_(std::make_unique<std::unordered_set<std::string>>())
            {
            }

            UserVerificator(std::vector<std::string>& white_list, std::vector<std::string>& black_list)
                : black_list_(std::make_unique<std::unordered_set<std::string>>
                    (black_list.begin(), black_list.end()))
                , white_list_(std::make_unique<std::unordered_set<std::string>>
                    (white_list.begin(), white_list.end()))
            {
            }

            bool Verify(const std::string_view user_name, const irc::domain::Role& role);

            void SetWhiteListOnly(bool status);
            void AddUserInWhiteList(std::string_view user_name);
            void RemoveUserFromWhiteList(std::string_view user_name);
            void AddUserInBlackList(std::string_view user_name);
            void RemoveUserFromBlackList(std::string_view user_name);
            void SetRoleLevel(int level);
            int GetRoleLevel();
            bool GetWhiteListOnly() const;
            std::unordered_set<std::string>* GetWhiteList();
            std::unordered_set<std::string>* GetBlackList();

        private:
            RoleFilter role_filter_;
            std::unique_ptr<std::unordered_set<std::string>> black_list_;
            std::unique_ptr<std::unordered_set<std::string>> white_list_;
            bool whitelist_only_ = false;

        };

    }

}