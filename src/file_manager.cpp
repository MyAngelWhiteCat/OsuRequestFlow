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

    void FileManager::WriteBinaryInRoot(std::string&& file_name, std::vector<char>&& bytes, bool save_history) {
        std::ofstream out(GetPathToFileInRoot(file_name), std::ios::binary);
        WriteInRoot(out, std::move(file_name), std::move(bytes), save_history);
    }

    void FileManager::WriteCharsInRoot(std::string&& file_name, std::vector<char>&& bytes, bool save_history) {
        std::ofstream out(GetPathToFileInRoot(file_name));
        WriteInRoot(out, std::move(file_name), std::move(bytes), save_history);
    }

    std::optional<std::vector<char>> FileManager::ReadCharsFromRoot(std::string_view file_name) {
        fs::path file_path = GetPathToFileInRoot(file_name);
        if (!fs::exists(file_path)) {
            return std::nullopt;
        }
        std::ifstream in(file_path); // todo error handling
        return ReadFromRoot(in, file_name);
    }

    std::optional<std::vector<char>> FileManager::ReadBinaryFromRoot(std::string_view file_name) {
        fs::path file_path = GetPathToFileInRoot(file_name);
        if (fs::exists(file_path)) {
            return std::nullopt;
        }
        if (!fs::is_regular_file(file_path)) {}
        std::ifstream in(file_path, std::ios::binary);
        return ReadFromRoot(in, file_name);
    }

    std::vector<char> FileManager::ReadFromRoot(std::ifstream& in, std::string_view file_name) {
        fs::path file = GetPathToFileInRoot(file_name);
        size_t size = fs::file_size(file);
        std::vector<char> bytes(size);
        in.read(bytes.data(), size);
        return bytes;
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

    void FileManager::WriteInRoot(std::ofstream& out, std::string&& file_name, std::vector<char>&& bytes, bool save_history) {
        try {
            Action act(ActionType::Write, std::move(file_name), root_directory_);
            out.write(bytes.data(), bytes.size());
            if (save_history) {
                AddAction(std::move(act));
                size_t bytes_writen = fs::file_size(actions_history_.back().GetFilePath());
                LOG_INFO("Successfuly write "
                    + std::to_string(bytes_writen) + " / " + std::to_string(bytes.size())
                    + " bytes to " + root_directory_.string());
            }
            else {
                LOG_INFO("Successfuly write " + std::to_string(bytes.size())
                    + " bytes to " + root_directory_.string());
            }
        }
        catch (const std::exception& e) {
            LOG_CRITICAL(e.what());
        }
    }

    void FileManager::AddAction(ActionType type, std::string&& file_name, const fs::path& path) {
        actions_history_.emplace_back(type, std::move(file_name), path);
    }

    void FileManager::AddAction(Action&& act) {
        actions_history_.push_back(std::move(act));
    }

    fs::path FileManager::GetPathToFileInRoot(std::string_view file_name) const {
        return root_directory_ / file_name;
    }

}