#include "user_validator.h"
#include <iostream>



void PrintSet(std::unordered_set<std::string>* list) {
    for (const auto& elem : *list) {
        std::cout << elem << "\n";
    }
}

int main() {
    {
        commands::user_validator::UserVerificator validator;
        validator.AddUserInWhiteList("myangelwhitecat");
        validator.AddUserInBlackList("justuser");
        validator.SetWhiteListOnly(true);
        validator.SetRoleLevel(1);

        std::cout << validator.GetRoleLevel() << std::endl;
        PrintSet(validator.GetBlackList());
        PrintSet(validator.GetWhiteList());
        validator.SaveSettings();
    }
    {
        commands::user_validator::UserVerificator validator;
        validator.AddUserInBlackList("r");
        validator.AddUserInBlackList("d");
        validator.AddUserInWhiteList("A");
        validator.SaveSettings();
        std::cout << validator.GetRoleLevel() << std::endl;
        PrintSet(validator.GetBlackList());
        PrintSet(validator.GetWhiteList());
    }

}