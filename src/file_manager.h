#pragma once

#include <filesystem>
#include <list>

#include <string_view>
#include <string>
#include <vector>
#include <fstream>
#include <optional>

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
            , path_(path)
        {

        }

        Action(ActionType type, std::string&& file_name, fs::path path)
            : type_(type)
            , file_name_(file_name)
            , path_(path)
        {

        }

        Action() = default;

        fs::path GetFilePath() {
            return fs::path(path_ / file_name_);
        }

        ActionType type_;
        std::string file_name_;
        fs::path path_;
    };

    class FileManager {
    public:
        FileManager(const fs::path& root_directory)
            : root_directory_(root_directory)
        {
            if (!fs::exists(root_directory)) {
                fs::create_directory(root_directory);
            }
        }

        void WriteBinaryInRoot(std::string&& file_name, std::vector<char>&& bytes, bool save_history = true);

        void WriteCharsInRoot(std::string&& file_name, std::vector<char>&& bytes, bool save_history = true);

        std::optional<std::vector<char>> ReadBinaryFromRoot(std::string_view file_name);

        std::optional<std::vector<char>> ReadCharsFromRoot(std::string_view file_name);


        void RemoveFile(const fs::path& path);

        void DeleteAllWritedFilesFromHistory();

    private:
        fs::path root_directory_;
        std::list<Action> actions_history_;

        std::vector<char> ReadFromRoot(std::ifstream& in, std::string_view file_name);

        void WriteInRoot(std::ofstream& out, std::string&& file_name, std::vector<char>&& bytes, bool save_history);

        void AddAction(ActionType type, std::string&& file_name, const fs::path& path);

        void AddAction(Action&& act);

        fs::path GetPathToFileInRoot(std::string_view file_name) const;
    };

}