#include "root_directory.h"

namespace root_directory {

    Content RootDirectory::GetContent(const fs::path& uri) {
        http::file_body::value_type file;

        auto decoded = fs::path(DecodeURL(uri.string()));

        decoded = fs::weakly_canonical(root_ / decoded);

        if (fs::is_directory(decoded)) {
            decoded = decoded / "index.html";
        }

        if (!fs::exists(decoded)) {
            return { std::move(file), ContentStatus::NOT_FOUND };
        }

        if (!IsSubdir(decoded)) {
            return { std::move(file), ContentStatus::BAD_ACCESS };
        }

        if (sys::error_code ec; file.open(decoded.string().c_str(), beast::file_mode::read, ec), ec) {
            throw std::runtime_error("Failed to open file "s.append(decoded.string()));
        }

        return { std::move(file), ContentStatus::OK };
    }

    bool RootDirectory::IsSubdir(fs::path path) const {
        path = fs::weakly_canonical(path);
        auto c_root = fs::weakly_canonical(root_);
        for (auto p = path.begin(), b = c_root.begin(); b != c_root.end(); ++p, ++b) {
            if (p == path.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

}