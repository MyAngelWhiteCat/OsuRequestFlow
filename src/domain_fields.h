#pragma once

#include <string_view>

namespace fields {

    using namespace std::literals;

    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv;
        constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
        constexpr static std::string_view TEXT_CSS = "text/css"sv;
        constexpr static std::string_view TEXT_JAVASCRIPT = "text/javascript"sv;
        constexpr static std::string_view APPLICATION_JSON = "application/json"sv;
        constexpr static std::string_view APPLICATION_XML = "application/xml"sv;
        constexpr static std::string_view IMAGE_PGN = "image/png"sv;
        constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
        constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
        constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
        constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
        constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
        constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;
        constexpr static std::string_view AUDIO_MPEG = "audio/mpeg"sv;
        constexpr static std::string_view UNKNOWN = "application/octet-stream"sv;
    };

    struct RequestError {
        RequestError() = delete;
        static constexpr std::string_view CODE = "code";
        static constexpr std::string_view MESSAGE = "message";
        static constexpr std::string_view MAP_NOT_FOUND = "mapNotFound";
        static constexpr std::string_view FILE_NOT_FOUND = "fileNotFound";
        static constexpr std::string_view BAD_REQUEST = "badRequest";
        static constexpr std::string_view INVALID_ARGUMENT = "invalidArgument";
        static constexpr std::string_view INVALID_METHOD = "invalidMethod";
    };

}