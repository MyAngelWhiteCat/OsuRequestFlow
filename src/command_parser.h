#pragma once

#include <optional>
#include <string>
#include <utility>
#include <string_view>

#include "message.h"
#include "command.h"
#include "logging.h"
#include "command_executor.h"

namespace commands {

    class CommandParser {
    public:
        CommandParser(CommandExecutor& executor)
            : executor_(executor)
        {

        }

        std::optional<Command> Parse(irc::domain::Message&& message) {
            auto line = message.GetContent();
            if (line.empty()) {
                return std::nullopt;
            }

            if (line[0] == command_start_) {
                // TODO:
            }
            if (auto id = CheckForLOsuMapURLAndGetID(line)) {
                Command osu_request(CommandType::OsuRequest, std::string(message.GetNick()), std::move(*id));
                LOG_INFO("Osu map find");
                return osu_request;
            }
            return std::nullopt;

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
        CommandExecutor& executor_;

        std::optional<std::string> CheckForLOsuMapURLAndGetID(std::string_view url) {
            size_t url_start = url.find(OSU_BEATMAPS_URL);
            LOG_INFO(std::to_string(url_start));
            if (url_start == std::string::npos) {
                return std::nullopt;
            }
            if (!CheckGameMode(url.substr(url_start))) {
                return std::nullopt;
            }
            return GetBeatmapSetId(url.substr(url_start));
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
            LOG_INFO("Checking for url " + std::string(url));
            std::string_view cut = url.substr(OSU_BEATMAPS_URL.size());
            size_t end = cut.find_first_of("#");
            return std::string(cut.substr(0, end));
        }

    };

}