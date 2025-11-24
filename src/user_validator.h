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
            irc::domain::Role accept_from_;
        };

        struct DomainNames {
            static constexpr std::string_view SETTINGS = "settings.cfg"sv;
            static constexpr std::string_view DELIMER = "-END-OF-LIST-"sv;
        };

        class UserVerificator {
        public:
            UserVerificator()
                : file_manager_(std::filesystem::current_path() / "users"s)
                , black_list_(std::make_unique<std::unordered_set<std::string>>())
                , white_list_(std::make_unique<std::unordered_set<std::string>>())
            {
                LoadSettings();
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

            std::unordered_set<std::string>* GetWhiteList() {
                return white_list_.get();
            }

            std::unordered_set<std::string>* GetBlackList() {
                return black_list_.get();
            }

            void SaveSettings() {
                std::vector<char> serialized;
                serialized.push_back(static_cast<char>(whitelist_only_));
                serialized.push_back('\n');
                serialized.push_back(role_filter_.GetLevel());
                serialized.push_back('\n');
                SerializeListTo(serialized, *white_list_);
                DelimSerialized(serialized, std::string(DomainNames::DELIMER));
                SerializeListTo(serialized, *black_list_);
                file_manager_.WriteCharsInRoot(std::string(DomainNames::SETTINGS), std::move(serialized), false);
            }

            void LoadSettings() {
                auto serialized = file_manager_.ReadCharsFromRoot(DomainNames::SETTINGS);
                if (!serialized || serialized->empty()) {
                    SaveSettings();
                    return;
                }
                whitelist_only_ = static_cast<bool>((*serialized)[0]);
                role_filter_.SetLevel((*serialized)[2]); // number 0 - 4;
                size_t start_of_white_list = 3;
                size_t start_of_black_list = LoadList(*serialized, white_list_.get(), start_of_white_list);
                LoadList(*serialized, black_list_.get(), start_of_black_list);
            }

        private:
            file_manager::FileManager file_manager_;
            RoleFilter role_filter_;
            std::unique_ptr<std::unordered_set<std::string>> black_list_;
            std::unique_ptr<std::unordered_set<std::string>> white_list_;
            bool whitelist_only_ = false;

            void SerializeListTo(std::vector<char>& input, std::unordered_set<std::string> list) {
                for (const auto& elem : list) {
                    for (const auto& ch : elem) {
                        input.push_back(ch);
                    }
                    input.push_back('\n');
                }
            }

            void DelimSerialized(std::vector<char>& input, std::string delimer) {
                for (const auto& ch : delimer) {
                    input.push_back(ch);
                }
                input.push_back('\n');
            }

            size_t LoadList(std::vector<char> chars, std::unordered_set<std::string>* list, size_t from) {
                std::string name;
                for (from; from < chars.size(); ++from) {
                    char& ch = chars[from];
                    if (ch == '\n') {
                        if (name == DomainNames::DELIMER) {
                            return ++from;
                        }
                        LOG_INFO(name);
                        list->insert(name);
                        name.clear();
                    }
                    else {
                        name += ch;
                    }
                }
                return 0;
            }
        };

    }

}