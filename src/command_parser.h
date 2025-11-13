#pragma once

#include <string>
#include <optional>


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
            if (auto id = CheckForLinkToOsuMapAndGetID(line)) {
                Command osu_request(CommandType::OsuRequest, std::move(*id));
            }
        }

        void SetCommandStart(char ch) {
            command_start_ = ch;
        }
    
    private:
        char command_start_ = '!';
        const std::string protocol_separator_ = "://";
        const std::string uri_separator_ = "/";
        const std::string OSU_DOMAIN = "osu.ppy.sh";
        const std::string OSUMAP_CATALOGUE = "beatmapsets";
        const std::string OSU_STD = "osu";



        std::optional<std::string> CheckForLinkToOsuMapAndGetID(std::string_view line) {
            std::string protocol = GetProtocol(line);
            if (protocol.empty()) {
                return std::nullopt;
            }
            std::string domain = GetDomain(line, protocol);
            if (domain.empty()) {
                return std::nullopt;
            }
            else if (domain != OSU_DOMAIN) {
                return std::nullopt;
            }
            std::string catalogue = GetCatalogue(line, protocol, domain);
            if (catalogue.empty()) {
                return std::nullopt;
            }
            else if (catalogue != OSUMAP_CATALOGUE) {
                return std::nullopt;
            }
            std::string game_mode = GetGameMode(line, protocol, domain, catalogue);
            if (game_mode.empty()) {
                return std::nullopt;
            }
            else if (game_mode != OSU_STD) {
                return std::nullopt;
            }
            std::string map_id = GetOsuMapID(line);
            if (map_id.empty()) {
                return std::nullopt;
            }
            return map_id;
        }

        std::string GetProtocol(std::string_view message) {
            size_t protocol_pos = message.find_first_of(':');
            if (protocol_pos != std::string::npos) {
                return std::string(message.substr(0, protocol_pos));
            }
            return "";
        }
        
        std::string GetDomain(std::string_view message, std::string_view protocol) {
            size_t start_pos = protocol.size() + protocol_separator_.size();
            size_t domain_pos = message.find_first_of('/', start_pos);
            if (domain_pos != std::string::npos) {
                return std::string(message.substr(start_pos, domain_pos - start_pos));
            }
            return "";
        }

        std::string GetCatalogue(std::string_view message, std::string_view protocol, std::string_view domain) {
            size_t start_pos = protocol.size() + protocol_separator_.size() + domain.size() + uri_separator_.size();
            size_t catalogue_pos = message.find_first_of('/', start_pos);
            if (catalogue_pos != std::string::npos) {
                return std::string(message.substr(start_pos, catalogue_pos - start_pos));
            }
            return "";
        }

        std::string GetGameMode(std::string_view message, std::string_view protocol, std::string_view domain, std::string_view catalogue) {
            size_t start_pos = protocol.size() + protocol_separator_.size() + domain.size() + uri_separator_.size();
            size_t game_mode_start_pos = message.find_first_of('#', start_pos);
            if (game_mode_start_pos == std::string::npos) {
                return "";
            }
            size_t game_mode_end_pos = message.find_first_of('/', game_mode_start_pos);
            if (game_mode_end_pos == std::string::npos) {
                return "";
            }

            return std::string(message.substr(game_mode_start_pos + 1, game_mode_end_pos - game_mode_start_pos - 1));
        }

        std::string GetOsuMapID(std::string_view message) {
            //"https://osu.ppy.sh/beatmapsets/2440776#osu/5324309"
            size_t map_id_pos = message.find_last_of('/');
            if (map_id_pos == std::string::npos) {
                return  "";
            }
            return std::string(message.substr(map_id_pos + 1, message.size()));
        }

    };

}