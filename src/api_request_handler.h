#pragma once

#include "core.h"
#include "response_maker.h"
#include "request_validator.h"
#include "chat_bot.h"

#include <string>
#include <string_view>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/fields.hpp>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <boost/beast/http/status.hpp>
#include <exception>
#include <boost/beast/http/verb.hpp>
#include "domain_fields.h"

namespace gui_http {

    using namespace std::literals;
    namespace beast = boost::beast;
    namespace http = beast::http;
    using json = nlohmann::json;

    struct APITarget {
        static constexpr std::string_view LOAD_SETTINGS = "/api/settings/load"sv;
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
        static constexpr std::string_view LAST_MESSAGES = "/api/vidget/chat/last_messages"sv;
        static constexpr std::string_view SHOW_CHAT = "/api/vidget/chat/show"sv;
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
            if (target == APITarget::LOAD_SETTINGS) {
                HandleLoadSetting(std::move(req), std::move(send));
            }
            else if (target == APITarget::SAVE_SETTINGS) {
                HandleSaveSetting(std::move(req), std::move(send));
            }
            else if (target == APITarget::SET_MAX_FILESIZE) {
                HandleSetMaxFileSize(std::move(req), std::move(send));
            }
            else if (target == APITarget::SET_DOWNLOADS_FOLDER) {
                HandleSetDownloadsFolder(std::move(req), std::move(send));
            }
            else if (target == APITarget::SET_DL_RESOURSE) {
                HandleSetDownloadsResourse(std::move(req), std::move(send));
            }
            else if (target == APITarget::WHITELIST) {
                HandleWhiteList(std::move(req), std::move(send));
            }
            else if (target == APITarget::BLACKLIST) {
                HandleBlackList(std::move(req), std::move(send));
            }
            else if (target == APITarget::ROLE_FILTER_LVL) {
                HandleSetRoleFilterLevel(std::move(req), std::move(send));
            }
            else if (target == APITarget::WHITELIST_ONLY) {
                HandleWhitelistOnly(std::move(req), std::move(send));
            }
            else if (target == APITarget::SET_RECONNECT_TIMEOUT) {
                HandleSetReconnectTimeout(std::move(req), std::move(send));
            }
            else if (target == APITarget::JOIN_CHANNEL) {
                HandleJoinChannel(std::move(req), std::move(send));
            } 
            else if (target == APITarget::PART_CHANNEL) {
                HandlePartChannel(std::move(req), std::move(send));
            }
            else if (target == APITarget::SHOW_CHAT) {
                HandleShowChat(std::move(req), std::move(send));
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
        
        // Settings

        template <typename Body, typename Allocator, typename Send>
        void HandleLoadSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        void HandleSaveSetting(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        // User Verificator

        template <typename Body, typename Allocator, typename Send>
        void HandleWhiteList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleBlackList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleSetRoleFilterLevel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        template <typename Body, typename Allocator, typename Send>
        void HandleWhitelistOnly(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        // Downloader

        template <typename Body, typename Allocator, typename Send>
        void HandleSetMaxFileSize(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        void HandleSetDownloadsFolder(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        void HandleSetDownloadsResourse(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        // IRC Client

        template <typename Body, typename Allocator, typename Send>
        void HandleSetReconnectTimeout(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        void HandleJoinChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        template <typename Body, typename Allocator, typename Send>
        void HandlePartChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);
        
        // Chat Widget

        template <typename Body, typename Allocator, typename Send>
        void HandleShowChat(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send);

        // Domain 

        template <typename Body, typename Allocator, typename Send>
        void SendWarning(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view warning);
        
        template <typename Body, typename Allocator, typename Send>
        void SendOK(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info);
        
        template <typename Body, typename Allocator, typename Send>
        void SendList(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::unordered_set<std::string>* list);

        template <typename Body, typename Allocator, typename Send>
        void SendServerError(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info);
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
        SendOK(req, send, "Settings loaded");
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
        SendOK(req, send, "Settings setted");
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
            SendOK(req, send, "Max file size updated");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetDownloadsFolder(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto path = request_validator_.ValidateDownloadsFolderRequest(req, send)) {
            core_.SetDownloadsDirectory(*path);
            SendOK(req, send, "Download folder updated");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetDownloadsResourse(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto resourse = request_validator_.ValidateSetResourseRequest(req, send)) {
            core_.SetDownloadResourseAndPrefix(resourse->first, resourse->second);
            SendOK(req, send, "Download resource updated");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleWhiteList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (req.method() == http::verb::get) {
            SendList(req, send, core_.GetWhiteList());
        }
        else if (req.method() == http::verb::put) {
            if (auto user = request_validator_.ValidateListEditRequest(req, send)) {
                if (core_.IsUserInBlackList(*user)) {
                    SendWarning(req, send, *user);
                    return;
                }
                try {
                    core_.AddUserInWhiteList(*user);
                    SendOK(req, send, "Add " + std::string(*user) + " in white list");
                }
                catch (const std::exception& e) {
                    SendServerError(req, send, e.what());
                    return;
                }
            }
        }
        else if (req.method() == http::verb::delete_) {
            if (auto user = request_validator_.ValidateListEditRequest(req, send)) {
                core_.RemoveUserFromWhiteList(*user);
                SendOK(req, send, "Remove " + std::string(*user) + " from white list");
            }
        }
        else {
            request_validator_.SendMethodNotAllowed(req, send, "GET, PUT, DELETE");
            return;
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleBlackList(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (req.method() == http::verb::get) {
            SendList(req, send, core_.GetBlackList());
        }
        else if (req.method() == http::verb::put) {
            if (auto user = request_validator_.ValidateListEditRequest(req, send)) {
                if (core_.IsUserInWhiteList(*user)) {
                    SendWarning(req, send, *user);
                    return;
                }
                core_.AddUserInBlackList(*user);
                SendOK(req, send, "Add " + std::string(*user) + " in black list");
            }
        }
        else if (req.method() == http::verb::delete_) {
            if (auto user = request_validator_.ValidateListEditRequest(req, send)) {
                core_.RemoveUserFromBlackList(*user);
                SendOK(req, send, "Remove " + std::string(*user) + " from black list");
            }
        }
        else {
            request_validator_.SendMethodNotAllowed(req, send, "GET, PUT, DELETE");
            return;
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetRoleFilterLevel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto level = request_validator_.ValidateSetRoleFilterRequest(req, send)) {
            core_.SetRoleLevelFilter(*level);
            SendOK(req, send, "Role filter level setted");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleWhitelistOnly(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto is_on = request_validator_.ValidateSetWhiteListOnlyRequest(req, send)) {
            core_.SetWhiteListOnly(*is_on);
            SendOK(req, send, "Updated white_list_only");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleSetReconnectTimeout(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto timeout = request_validator_.ValidateReconnectTimeoutRequest(req, send)) {
            core_.SetReconnectTimeout(*timeout);
            SendOK(req, send, "Reconnect timeout updated");
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleJoinChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto channel = request_validator_.ValidateJoinRequest(req, send)) {
            core_.Join(*channel);
            SendOK(req, send, "Join " + std::string(*channel));
        }
    }

    template <typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandlePartChannel(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto channel = request_validator_.ValidatePartRequest(req, send)) {
            core_.Part(*channel);
            SendOK(req, send, "Part " + std::string(*channel));
        }
    }

    template<typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::HandleShowChat(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        if (auto toggle = request_validator_.ValidateShowChatRequest(req, send)) {
            core_.ShowChat(*toggle);
            SendOK(req, send, "OK");
        }
    }

    template<typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::SendWarning(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view warning) {
        send(response_maker_.MakeStringResponse(
            http::status::conflict, // 409
            req.version(),
            response_maker_.MakeBadRequest("Warning", warning),
            req.keep_alive()
        ));
    }

    template<typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::SendOK(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info) {
        send(response_maker_.MakeStringResponse(
            http::status::ok,
            req.version(),
            info,
            req.keep_alive())
        );
    }

    template<typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::SendList(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::unordered_set<std::string>* list) {
        json serialized = json::array();
        for (auto it = list->begin(); it != list->end(); ++it) {
            serialized.push_back(*it);
        }
        send(response_maker_.MakeStringResponse(
            http::status::ok,
            req.version(),
            std::move(serialized),
            req.keep_alive()
        ));
    }

    template<typename Body, typename Allocator, typename Send>
    inline void ApiRequestHandler::SendServerError(http::request<Body, http::basic_fields<Allocator>>& req, Send&& send, std::string_view info) {
        send(response_maker_.MakeStringResponse(
            http::status::internal_server_error,
            req.version(),
            response_maker_.MakeBadRequest("Fatal server error", info),
            req.keep_alive())
        );
    }

}