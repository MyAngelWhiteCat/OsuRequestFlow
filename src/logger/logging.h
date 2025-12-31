#pragma once
// AI on
#include <spdlog/spdlog.h>


#define LOG_INFO(message) logging::Logger::Info(message)
#define LOG_ERROR(message) logging::Logger::Error(message)
#define LOG_WARN(message) logging::Logger::Warn(message)
#define LOG_DEBUG(message) logging::Logger::Debug(message)
#define LOG_TRACE(message) logging::Logger::Trace(message)
#define LOG_CRITICAL(message) logging::Logger::Critical(message)
#define LOG_FUCKUP(message) logging::Logger::Critical(message)

namespace logging {

    class Logger {
    public:
        static void Init();
        static void Shutdown();

        static void Info(const std::string& message) {
            spdlog::info(message);
        }

        static void Error(const std::string& message) {
            spdlog::error(message);
        }

        static void Warn(const std::string& message) {
            spdlog::warn(message);
        }

        static void Debug(const std::string& message) {
            spdlog::debug(message);
        }

        static void Trace(const std::string& message) {
            spdlog::trace(message);
        }

        static void Critical(const std::string& message) {
            spdlog::critical(message);
        }
    };

    template <typename ErrorCode>
    static void ReportError(const ErrorCode& ec, std::string_view where) {
        spdlog::error("Error in {}: {} (code: {})",
            where, ec.message(), ec.value());
    }

} // namespace logging

// AI off