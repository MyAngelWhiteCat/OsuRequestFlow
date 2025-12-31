#pragma once

#include <string>
#include <string_view>


static std::string DecodeURL(const std::string_view uri) {
    std::string decoded;
    std::string uri_str(uri);

    for (int i = 0; i < uri_str.size(); ++i) {
        char ch = uri_str[i];
        if (ch == '%') {
            char hex[2] = { uri_str[++i], uri_str[++i] };
            int char_code = std::stoi(hex, nullptr, 16);
            ch = static_cast<char>(char_code);
        }
        decoded += ch;
    }

    if (!decoded.empty() && decoded[0] == '/') {
        decoded.erase(0, 1);
    }

    return std::string(decoded);
}