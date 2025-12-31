#include "gui/response_maker.h"
#include "http_server/domain_fields.h"

namespace response_maker {

    json ResponseMaker::GetBadRequest() {
        json obj;
        obj[fields::RequestError::CODE] = fields::RequestError::BAD_REQUEST;
        obj[fields::RequestError::MESSAGE] = "Bad request"s;
        return obj;
    }

    json ResponseMaker::MakeBadRequest(std::string_view key, std::string_view value) {
        json obj;
        obj[fields::RequestError::CODE] = std::string(key);
        obj[fields::RequestError::MESSAGE] = std::string(value);
        return obj;
    }

    std::string_view ResponseMaker::GetContentType(const std::filesystem::path& uri) {
        if (fs::is_directory(uri)) {
            return fields::ContentType::TEXT_HTML;
        }

        std::string uri_str = uri.string();
        std::string file_extension = uri_str.substr(uri_str.find('.') + 1, uri_str.size());

        if (file_extension == "htm" || file_extension == "html") {
            return fields::ContentType::TEXT_HTML;
        }

        if (file_extension == "css") {
            return fields::ContentType::TEXT_CSS;
        }

        if (file_extension == "txt") {
            return fields::ContentType::TEXT_PLAIN;
        }

        if (file_extension == "js") {
            return fields::ContentType::TEXT_JAVASCRIPT;
        }

        if (file_extension == "json") {
            return fields::ContentType::APPLICATION_JSON;
        }

        if (file_extension == "xml") {
            return fields::ContentType::APPLICATION_XML;
        }

        if (file_extension == "png") {
            return fields::ContentType::IMAGE_PGN;
        }

        if (file_extension == "jpeg" || file_extension == "jpe" || file_extension == "jpg") {
            return fields::ContentType::IMAGE_JPEG;
        }

        if (file_extension == "gif") {
            return fields::ContentType::IMAGE_GIF;
        }

        if (file_extension == "bmp") {
            return fields::ContentType::IMAGE_BMP;
        }

        if (file_extension == "ico") {
            return fields::ContentType::IMAGE_ICO;
        }

        if (file_extension == "tiff" || file_extension == "tif") {
            return fields::ContentType::IMAGE_TIFF;
        }

        if (file_extension == "svg" || file_extension == "svgz") {
            return fields::ContentType::IMAGE_SVG;
        }

        if (file_extension == "mp3") {
            return fields::ContentType::AUDIO_MPEG;
        }

        return fields::ContentType::UNKNOWN;
    }

}
