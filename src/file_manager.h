#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <list>

#include "logging.h"

namespace file_manager {

    namespace fs = std::filesystem;

    enum class ActionType {
        Write,
        Delete
    };

    struct Action {
        Action(ActionType type, std::string_view file_name, std::string_view file_format, fs::path path)
            : type_(type)
            , file_name_(file_name)
            , file_format_(file_format)
            , path_(path)
        {

        }

        ActionType type_;
        std::string file_name_;
        std::string file_format_;
        fs::path path_;
    };

    class FileManager {
    public:
        FileManager(const fs::path& root_directory)
            : root_directory_(root_directory)
        {

        }

        void WriteInRoot(const std::vector<char>& bytes, std::string_view file_name, std::string_view format) {
            std::ofstream new_file(root_directory_.string() + std::string(file_name) + std::string(format));
            for (const char ch : bytes) {
                new_file << ch;
            }
            AddAction(ActionType::Write, file_name, format, root_directory_);
        }

        void DeleteFile(const fs::path& path) {
            try {
                fs::remove(path);
            }
            catch (const std::exception& e) {
                LOG_ERROR(e.what());
            }
        }

    private:
        fs::path root_directory_;
        std::list<Action> actions_history_;

        void AddAction(ActionType type, std::string_view file_name_, std::string_view file_format, const fs::path& path) {
            actions_history_.emplace_back(type, file_name_, file_format, path);
        }
    };


}