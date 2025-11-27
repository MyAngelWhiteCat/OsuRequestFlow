#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <boost/beast.hpp>
#include <iostream>

#include "decode_url.h"


namespace root_directory {

    namespace fs = std::filesystem;
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace sys = boost::system;

    using namespace std::literals;

    enum class ContentStatus {
        OK,
        NOT_FOUND,
        BAD_ACCESS
    };

    struct Content {
        http::file_body::value_type file;
        ContentStatus status;
    };

    class RootDirectory {
    public:
        explicit RootDirectory(const fs::path& root)
            : root_(fs::weakly_canonical(root))
        {

        }

        Content GetContent(const fs::path& uri);

    private:
        fs::path root_;

        bool IsSubdir(fs::path path) const;

        fs::path DecodeURL(const fs::path& uri) const;
    };

}