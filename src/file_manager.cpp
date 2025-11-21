#include "file_manager.h"
#include "logging.h"
#include <filesystem>
#include <fstream>
#include <list>

#include <utility>
#include <exception>
#include <string_view>
#include <string>
#include <vector>

namespace file_manager {

    namespace fs = std::filesystem;

    void FileManager::WriteInRoot(std::string&& file_name, std::vector<char>&& bytes) {
        try {
            Action act(ActionType::Write, std::move(file_name), root_directory_);
            std::ofstream new_file(root_directory_.string() + "\\" + act.file_name_, std::ios::binary);
            new_file.write(bytes.data(), bytes.size());
            AddAction(std::move(act));
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }

        size_t bytes_writen = fs::file_size(actions_history_.back().GetFilePath());
        LOG_INFO("Successfuly write "
            + std::to_string(bytes_writen) + " / " + std::to_string(bytes.size())
            + " bytes to " + root_directory_.string());
    }

    void FileManager::RemoveFile(const fs::path& path) {
        if (fs::remove(path)) {
            LOG_INFO("Deleted " + path.string());
        }
        else {
            LOG_ERROR("Cant delete " + path.string());
        }
    }

    void FileManager::DeleteAllWritedFilesFromHistory() {
        auto elem = actions_history_.begin();
        while (elem != actions_history_.end()) {
            Action& act = *elem;
            if (act.type_ == ActionType::Write) {
                RemoveFile(fs::path(act.path_.string() + '/' + act.file_name_));
                elem = actions_history_.erase(elem);
            }
            else {
                ++elem;
            }
        }
    }

    void FileManager::AddAction(ActionType type, std::string&& file_name, const fs::path& path) {
        actions_history_.emplace_back(type, std::move(file_name), path);
    }

    void FileManager::AddAction(Action&& act) {
        actions_history_.push_back(std::move(act));
    }

}