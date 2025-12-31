#include "osu_file_manager.h"
#include "logger/logging.h"

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

    void OsuFileManager::RemoveFileFromRoot(std::string_view file_name) {
        RemoveFile(root_directory_ / file_name);
    }

    void OsuFileManager::RemoveFile(const fs::path& path) {
        if (fs::remove(path)) {
            LOG_INFO("Deleted " + path.string());
        }
        else {
            LOG_ERROR("Cant delete " + path.string());
        }
    }

    void OsuFileManager::RemoveDirectory_(const fs::path& path) {
        if (fs::remove_all(path)) {
            LOG_INFO("Deleted " + path.string());
        }
        else {
            LOG_ERROR("Cant delete " + path.string());
        }
    }

    void OsuFileManager::RemoveDuplicates() {
        std::unordered_map<std::string, fs::path> id_to_path;
        for (const auto& dir_entry : fs::directory_iterator(root_directory_)) {
            std::string map_id = std::string(GetMapIdFromMapFolderName(dir_entry.path().string()));
            if (auto it = id_to_path.find(map_id); it !=  id_to_path.end()) {
                if (dir_entry.path().string().size() < it->second.string().size()) {
                    id_to_path[map_id] = dir_entry.path();
                } 
            }
            else {
                id_to_path[map_id] = dir_entry.path();
            }
        }

        for (const auto& dir_entry : fs::directory_iterator(root_directory_)) {
            std::string map_id = std::string(GetMapIdFromMapFolderName(dir_entry.path().string()));
            if (auto it = id_to_path.find(map_id); it != id_to_path.end()) {
                if (dir_entry.path().string().size() > it->second.string().size()) {
                    if (dir_entry.is_directory()) {
                        RemoveDirectory_(dir_entry.path());
                    }
                    else {
                        RemoveFile(dir_entry.path());
                    }
                }
            }

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
        size_t id_end = folder_name.find_first_of(' ');
        auto temp = folder_name.substr(0, id_end);
        size_t id_start = folder_name.find_last_of('\\');
        auto id = temp.substr(id_start + 1, id_end);
        LOG_INFO(std::string(id));
        for (auto ch : id) {
            if (!std::isdigit(ch)) {
                throw std::runtime_error("Map id should be number");
            }
        }
        return id;
    }


}