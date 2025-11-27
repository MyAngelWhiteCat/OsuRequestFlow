#pragma once

#include "file_manager.h"
#include "message.h" // !! Role only required!!!
#include "logging.h"

#include <string>
#include <string_view>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <vector>
#include <utility>


namespace commands {

    namespace user_validator {

        using namespace std::literals;

        class RoleFilter {
        public:
            bool CheckRole(irc::domain::Role role) {
                return role >= accept_from_;
            }

            void SetFollowers() {
                accept_from_ = irc::domain::Role::FOLLOWER;
            }

            void SetSubsribers() {
                accept_from_ = irc::domain::Role::SUBSCRIBER;
            }

            void SetVIPs() {
                accept_from_ = irc::domain::Role::VIP;
            }

            void SetMods() {
                accept_from_ = irc::domain::Role::MODERATOR;
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

            bool Verify(const std::string_view user_name, const irc::domain::Role& role) {
                if (black_list_->count(std::string(user_name))) {
                    return false;
                }

                if (whitelist_only_) {
                    return white_list_->count(std::string(user_name));
                }

                return role_filter_.CheckRole(role);
            }

            void SetWhiteListOnly(bool status) {
                whitelist_only_ = status;
            }

            void AddUserInWhiteList(std::string_view user_name) {
                white_list_->insert(std::string(user_name));
            }

            void RemoveUserFromWhiteList(std::string_view user_name) {
                white_list_->erase(std::string(user_name));
            }

            void AddUserInBlackList(std::string_view user_name) {
                black_list_->insert(std::string(user_name));
            }

            void RemoveUserFromBlackList(std::string_view user_name) {
                black_list_->erase(std::string(user_name));
            }

            void SetRoleLevel(int level) {
                role_filter_.SetLevel(level);
            }

            int GetRoleLevel() {
                return role_filter_.GetLevel();
            }

            bool GetWhiteListOnly() const {
                return whitelist_only_;
            }

            std::unordered_set<std::string>* GetWhiteList() {
                return white_list_.get();
            }

            std::unordered_set<std::string>* GetBlackList() {
                return black_list_.get();
            }

        private:
            RoleFilter role_filter_;
            std::unique_ptr<std::unordered_set<std::string>> black_list_;
            std::unique_ptr<std::unordered_set<std::string>> white_list_;
            bool whitelist_only_ = false;

        };

    }

}