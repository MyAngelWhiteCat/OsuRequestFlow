#pragma once

#include <optional>
#include <string>
#include <utility>


namespace commands {

    enum class CommandType {
        OsuRequest
    };

    struct Command {
    public:
        Command(CommandType type, std::string&& content)
            : type_(type)
            , content_(std::move(content))
        {

        }

        CommandType type_;
        std::string content_;
    };

    class CommandParser {
    public:
        std::optional<Command> GetCommand(std::string line) {
            if (line.empty()) {
                return std::nullopt;
            }

            if (line[0] == command_start_) {
                // TODO:
            }
            if (auto id = CheckForLOsuMapURLAndGetID(line)) {
                Command osu_request(CommandType::OsuRequest, std::move(*id));
            }
        }

        void SetCommandStart(char ch) {
            command_start_ = ch;
        }

        void SetGameMode(std::string_view mode) {
            OSU_GAME_MODE = std::string(mode);
        }
    
    private:
        char command_start_ = '!';
        const std::string OSU_BEATMAPS_URL = "https://osu.ppy.sh/beatmapsets/";
        std::string OSU_GAME_MODE = "osu";

        std::optional<std::string> CheckForLOsuMapURLAndGetID(std::string_view url) {
            if (url.substr(0, OSU_BEATMAPS_URL.size()) != OSU_BEATMAPS_URL) {
                return std::nullopt;
            }
            if (!CheckGameMode(url)) {
                return std::nullopt;
            }
            return GetBeatmapSetId(url);
        }

        bool CheckGameMode(std::string_view url) {
            std::string_view cut = url.substr(OSU_BEATMAPS_URL.size());
            size_t start = cut.find_first_of("#") + 1;
            size_t end = cut.find_first_of("/");
            return url.substr(start, end - start) == OSU_GAME_MODE;
        }

        std::string GetOsuMapID(std::string_view message) {
            //"https://osu.ppy.sh/beatmapsets/2440776#osu/5324309"
            size_t map_id_pos = message.find_last_of('/');
            if (map_id_pos == std::string::npos) {
                return  "";
            }
            return std::string(message.substr(map_id_pos + 1, message.size()));
        }

        std::string GetBeatmapSetId(std::string_view url) {
            std::string_view cut = url.substr(OSU_BEATMAPS_URL.size());
            size_t end = cut.find_first_of("#");
            return std::string(cut.substr(0, end));
        }

    };

}