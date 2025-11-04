#include "client.h"

namespace irc {

    void ReportError(sys::error_code ec, std::string_view where) {
        std::string message = fmt::format("Error in {}: {} (code: {})",
            where, ec.message(), ec.value());
        LOG_ERROR(message);
    }

} // namespace irc