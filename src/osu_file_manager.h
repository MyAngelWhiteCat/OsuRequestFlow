#pragma once

#include <filesystem>
#include <list>

#include <string_view>
#include <string>
#include <vector>
#include <fstream>
#include <optional>

namespace osu_file_manager {

    namespace fs = std::filesystem;


    class OsuFileManager {
    public:
        OsuFileManager(const fs::path& root_directory)
            : root_directory_(root_directory)
        {
            if (!fs::exists(root_directory)) {
                fs::create_directory(root_directory);
            }
        }
        fs::path GetRootDirectory() const;
        void RemoveFile(const fs::path& path);
        void RemoveDirectory(const fs::path& path);
        void RemoveDuplicates();
        bool IsAlreadyInstalled(std::string_view map_id);

    private:
        fs::path root_directory_;

        std::string_view GetMapIdFromMapFolderName(std::string_view folder_name) const;
        
    };

}