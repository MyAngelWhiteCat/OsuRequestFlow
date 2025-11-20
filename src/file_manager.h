#pragma once

#include <filesystem>
#include <fstream>
#include <list>

#include "logging.h"
#include <utility>
#include <exception>
#include <string_view>
#include <string>
#include <vector>

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

        }

        void WriteInRoot(std::vector<char>&& bytes, std::string&& file_name) {
            try {
                Action act(ActionType::Write, std::move(file_name), root_directory_);
                std::ofstream new_file(root_directory_.string() + "\\" + act.file_name_);
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

        void RemoveFile(const fs::path& path) {
            if (fs::remove(path)) {
                LOG_INFO("Deleted " + path.string());
            }
            else {
                LOG_ERROR("Cant delete " + path.string());
            }
        }

        void DeleteAllWritedFilesFromHistory() {
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

    private:
        fs::path root_directory_;
        std::list<Action> actions_history_;

        void AddAction(ActionType type, std::string&& file_name, const fs::path& path) {
            actions_history_.emplace_back(type, std::move(file_name), path);
        }

        void AddAction(Action&& act) {
            actions_history_.push_back(std::move(act));
        }
    };


}