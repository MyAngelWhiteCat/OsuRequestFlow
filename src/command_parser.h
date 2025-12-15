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

        std::optional<Command> Parse(irc::domain::Message&& message);
        void SetCommandStart(char ch);
        void SetGameMode(std::string_view mode);
    
    private:
        char command_start_ = '!';
        const std::string OSU_BEATMAPS_URL = "https://osu.ppy.sh/beatmapsets/";
        std::string OSU_GAME_MODE = "osu";
        CommandExecutor& executor_;

        std::optional<std::string> CheckForOsuMapURLAndGetID(std::string_view url);
        bool CheckGameMode(std::string_view url);
        std::string GetOsuMapID(std::string_view message);
        std::string GetBeatmapSetId(std::string_view url);
    };

}