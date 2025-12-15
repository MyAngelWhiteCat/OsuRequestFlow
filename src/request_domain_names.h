#pragma once

#include <string>
#include <string_view>

namespace gui_http {

    using namespace std::literals;

    struct APITarget {
        static constexpr std::string_view LOAD_SETTINGS = "/api/settings/load"sv;
        static constexpr std::string_view SAVE_SETTINGS = "/api/settings/save"sv;
        static constexpr std::string_view MAX_FILESIZE = "/api/downloader/settings/max_file_size"sv;
        static constexpr std::string_view MESURE_SPEED = "/api/downloader/mesure_speed"sv;
        static constexpr std::string_view DL_SERVER_STATUS = "/api/downloader/dl_server_status"sv;
        static constexpr std::string_view DOWNLOADS_FOLDER = "/api/downloader/settings/folder"sv;
        static constexpr std::string_view BASE_SERVERS = "/api/downloader/base_servers"sv;
        static constexpr std::string_view DL_RESOURCE = "/api/downloader/settings/resource_and_prefix"sv;
        static constexpr std::string_view WHITELIST = "/api/white_list/users"sv;
        static constexpr std::string_view BLACKLIST = "/api/black_list/users"sv;
        static constexpr std::string_view ROLE_FILTER_LVL = "/api/validator/settings/role_filter_level"sv;
        static constexpr std::string_view WHITELIST_ONLY = "/api/validator/settings/whitelist_only"sv;
        static constexpr std::string_view RECONNECT_TIMEOUT = "/api/irc_client/settings/reconnect_timeout"sv;
        static constexpr std::string_view JOIN_CHANNEL = "/api/irc_client/join"sv;
        static constexpr std::string_view PART_CHANNEL = "/api/irc_client/part"sv;
        static constexpr std::string_view LAST_MESSAGES = "/api/widget/chat/last_messages"sv;
        static constexpr std::string_view SHOW_CHAT = "/api/widget/chat/show"sv;
    };

    struct Settings {
        static constexpr std::string_view ROLE_FILTER_LEVEL = "RoleFilterLevel"sv;
        static constexpr std::string_view FILE_SIZE = "FileSize"sv;
        static constexpr std::string_view PATH = "Path"sv;
        static constexpr std::string_view USER_NAME = "UserName"sv;
        static constexpr std::string_view CHANNEL = "Channel"sv;
        static constexpr std::string_view Resource = "Resource"sv;
        static constexpr std::string_view PREFIX = "Prefix"sv;
        static constexpr std::string_view RECONNECT_TIMEOUT = "ReconnectTimeout"sv;
        static constexpr std::string_view ENABLED = "Enabled"sv;
        static constexpr std::string_view IS_MESURED = "IsMesured"sv;
        static constexpr std::string_view IS_ANY_AVAILABLE = "IsAnyAvailable"sv;
        static constexpr std::string_view STATUS = "Status"sv;
    };
}