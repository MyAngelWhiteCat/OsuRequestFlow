#pragma once

#include "core.h"
#include "response_maker.h"
#include "request_validator.h"

#include <string>
#include <string_view>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/fields.hpp>

namespace gui_http {

    using namespace std::literals;
    namespace beast = boost::beast;
    namespace http = beast::http;


    struct ApiEps {
        static constexpr std::string_view LOAD_SETTINGS = "/api/setting/load"sv;
        static constexpr std::string_view SAVE_SETTINGS = "/api/settings/save"sv;
        static constexpr std::string_view SET_MAX_FILESIZE = "/api/downloader/settings/max_file_size"sv;
        static constexpr std::string_view SET_DOWNLOADS_FOLDER = "/api/downloader/settings/folder"sv;
        static constexpr std::string_view SET_DL_RESOURSE = "/api/downloader/settings/resourse_and_prefix"sv;
        static constexpr std::string_view WHITELIST = "/api/white_list/users"sv;
        static constexpr std::string_view BLACKLIST = "/api/black_list/users"sv;
        static constexpr std::string_view ROLE_FILTER_LVL = "/api/validator/settings/role_filter_level"sv;
        static constexpr std::string_view WHITELIST_ONLY = "/api/validator/settings/set_whitelist_only"sv;
        static constexpr std::string_view SET_RECONNECT_TIMEOUT = "/api/irc_client/settings/set_reconnect_timeout"sv;
        static constexpr std::string_view JOIN_CHANNEL = "/api/irc_client/join"sv;
        static constexpr std::string_view PART_CHANNEL = "/api/irc_client/part"sv;
    };

    class ApiRequestHandler {
    public:
        ApiRequestHandler(core::Core& core)
            : core_(core)
        {
        }

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            std::string_view target = req.target();
            if (target == ApiEps::LOAD_SETTINGS) {
                HandleLoadSetting(std::move(req), std::move(send));
            }
            else if (target == ApiEps::SAVE_SETTINGS) {
                HandleSaveSetting(std::move(req), std::move(send));
            }
            else if (target == ApiEps::SET_MAX_FILESIZE) {
                HandleSetMaxFileSize(std::move(req), std::move(send));
            }
            else if (target == ApiEps::SET_DOWNLOADS_FOLDER) {
                HandleSetDownloadsFolder(std::move(req), std::move(send));
            }
            else if (target == ApiEps::SET_DL_RESOURSE) {
                HandleSetDownloadsResourse(std::move(req), std::move(send));
            }
            else if (target == ApiEps::WHITELIST) {
                HandleWhiteList(std::move(req), std::move(send));
            }
            else if (target == ApiEps::BLACKLIST) {
                HandleBlackList(std::move(req), std::move(send));
            }
            else if (target == ApiEps::ROLE_FILTER_LVL) {
                HandleSetRoleFilterLevel(std::move(req), std::move(send));
            }
            else if (target == ApiEps::WHITELIST_ONLY) {
                HandleWhitelistOnly(std::move(req), std::move(send));
            }
            else if (target == ApiEps::SET_RECONNECT_TIMEOUT) {
                HandleSetReconnectTimeout(std::move(req), std::move(send));
            }
            else if (target == ApiEps::JOIN_CHANNEL) {
                HandleJoinChannel(std::move(req), std::move(send));
            } 
            else if (target == ApiEps::PART_CHANNEL) {
                HandlePartChannel(std::move(req), std::move(send));
            }
            else {
                HandleWrongEndPoint(std::move(req), std::move(send));
            }
        }

    private:
        core::Core& core_;
        response_maker::ResponseMaker response_maker_;
        request_validator::RequestValidator request_validator_{ response_maker_ };

        template <typename Body, typename Allocator, typename Send>
        void HandleWrongEndPoint(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleLoadSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSaveSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetMaxFileSize(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetDownloadsFolder(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetDownloadsResourse(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleWhiteList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleBlackList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetRoleFilterLevel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleWhitelistOnly(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetReconnectTimeout(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleJoinChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandlePartChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
    };

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleWrongEndPoint(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        send(response_maker_.MakeStringResponse(
                http::status::not_found,
                req.version(),
                response_maker_.MakeBadRequest(fields::RequestError::BAD_REQUEST, "Not Found"),
                req.keep_alive())
        );
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleLoadSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (!request_validator_.ValidateSettingsRequest(req, send)) {
            return;
        }

        try {
            core_.LoadSettings();
        }
        catch (const std::exception& e) {
            send(response_maker_.MakeStringResponse(
                http::status::internal_server_error,
                req.version(),
                response_maker_.MakeBadRequest(fields::RequestError::FILE_NOT_FOUND, e.what()),
                req.keep_alive())
            );
        }
        send(response_maker_.MakeStringResponse(
            http::status::ok,
            req.version(),
            "Settings loaded",
            req.keep_alive())
        );
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSaveSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (!request_validator_.ValidateSettingsRequest(req, send)) {
            return;
        }

        try {
            core_.SaveSettings();
        }
        catch (const std::exception& e) {
            send(response_maker_.MakeStringResponse(
                http::status::internal_server_error,
                req.version(),
                response_maker_.MakeBadRequest(fields::RequestError::FILE_NOT_FOUND, e.what()),
                req.keep_alive())
            );
        }
        send(response_maker_.MakeStringResponse(
            http::status::ok,
            req.version(),
            "Settings saved",
            req.keep_alive())
        );
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetMaxFileSize(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto new_size = request_validator_.ValidateFileSizeRequest(req, send)) {
            try {
                core_.SetMaxFileSize(*new_size);
            }
            catch (const std::exception& e) {
                send(response_maker_.MakeStringResponse(
                    http::status::internal_server_error,
                    req.version(),
                    response_maker_.MakeBadRequest(fields::RequestError::FILE_NOT_FOUND, e.what()),
                    req.keep_alive())
                );
            }
            send(response_maker_.MakeStringResponse(
                http::status::ok,
                req.version(),
                "Max file size updated",
                req.keep_alive())
            );
        }
        
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetDownloadsFolder(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (!request_validator_.ValidateDownloadsFolderRequest(req, send)) {
            return;
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetDownloadsResourse(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleWhiteList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleBlackList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetRoleFilterLevel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleWhitelistOnly(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetReconnectTimeout(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleJoinChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandlePartChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

    }

}