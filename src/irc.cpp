#include "irc.h"

namespace irc {

    void ReportError(sys::error_code ec, std::string_view where) {
        auto code = ec.value();
        auto message = ec.message();
        std::cout << "[ERROR] While: " << where << " | with code: "
            << code << " | because: " << message << std::endl;
    }

} // namespace irc

