#include "osu_file_manager.h"
#include "logging.h"
#include <filesystem>
#include <fstream>
#include <list>

#include <utility>
#include <exception>
#include <string_view>
#include <string>
#include <vector>

namespace osu_file_manager {

    namespace fs = std::filesystem;

    fs::path OsuFileManager::GetRootDirectory() const {
        return root_directory_;
    }

    void OsuFileManager::RemoveFile(const fs::path& path) {
        if (fs::remove(path)) {
            LOG_INFO("Deleted " + path.string());
        }
        else {
            LOG_ERROR("Cant delete " + path.string());
        }
    }

    void OsuFileManager::RemoveDirectory(const fs::path& path) {
        if (fs::remove_all(path)) {
            LOG_INFO("Deleted " + path.string());
        }
        else {
            LOG_ERROR("Cant delete " + path.string());
        }
    }

    void OsuFileManager::RemoveDuplicates() {
        bool is_first = true;
        bool is_skiped = true;
        for (const auto& dir_entry : fs::directory_iterator(root_directory_)) {
            const auto& first = dir_entry.path();
            for (const auto& sec_dir_entry : fs::directory_iterator(root_directory_)) {
                if (is_first) {
                    is_skiped = true;
                    is_first = false;
                    continue;
                }
                const auto& second = sec_dir_entry.path();
                try {
                    std::string_view first_id = GetMapIdFromMapFolderName(first.string());
                    std::string_view second_id = GetMapIdFromMapFolderName(second.string());
                    if (first_id == second_id) {
                        if (fs::is_directory(second)) {
                            RemoveDirectory(second);
                        }
                        else {
                            RemoveFile(second);
                        }
                    }
                }
                catch (const std::exception& e) {
                    LOG_CRITICAL(e.what());
                }
            }
            if (!is_skiped) {
                is_first = true;
            }
            is_skiped = false;
        }

    }

    bool OsuFileManager::IsAlreadyInstalled(std::string_view map_id) {
        for (const auto& dir_entry : fs::directory_iterator(root_directory_)) {
            if (GetMapIdFromMapFolderName(dir_entry.path().string()) == map_id) {
                return true;
            }
        }
        return false;
    }

    std::string_view OsuFileManager::GetMapIdFromMapFolderName(std::string_view folder_name) const {
        auto id = folder_name.substr(folder_name.find_first_of(' '));
        for (auto ch : id) {
            if (!std::isdigit(ch)) {
                throw std::runtime_error("Map id should be number");
            }
        }
        return id;
    }


}