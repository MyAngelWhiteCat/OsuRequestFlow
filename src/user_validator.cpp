#include "user_validator.h"



namespace commands {

    namespace user_validator {

        bool UserVerificator::Verify(const std::string_view user_name, const irc::domain::Role& role) {
            if (black_list_->count(std::string(user_name))) {
                return false;
            }
            if (white_list_->count(std::string(user_name))) {
                return true;
            }
            if (whitelist_only_) {
                return false;
            }
            return role_filter_.CheckRole(role);
        }

        void UserVerificator::SetWhiteListOnly(bool status) {
            whitelist_only_ = status;
        }

        void UserVerificator::AddUserInWhiteList(std::string_view user_name) {
            std::cout << "Adding to white list " << user_name << std::endl;
            white_list_->insert(std::string(user_name));
        }

        void UserVerificator::RemoveUserFromWhiteList(std::string_view user_name) {
            white_list_->erase(std::string(user_name));
        }

        void UserVerificator::AddUserInBlackList(std::string_view user_name) {
            black_list_->insert(std::string(user_name));
        }

        void UserVerificator::RemoveUserFromBlackList(std::string_view user_name) {
            black_list_->erase(std::string(user_name));
        }

        void UserVerificator::SetRoleLevel(int level) {
            role_filter_.SetLevel(level);
        }

        int UserVerificator::GetRoleLevel() {
            return role_filter_.GetLevel();
        }

        bool UserVerificator::GetWhiteListOnly() const {
            return whitelist_only_;
        }

        std::unordered_set<std::string>* UserVerificator::GetWhiteList() {
            return white_list_.get();
        }

        std::unordered_set<std::string>* UserVerificator::GetBlackList() {
            return black_list_.get();
        }

    }

}