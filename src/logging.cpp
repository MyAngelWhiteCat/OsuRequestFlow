#include "logging.h"


namespace logging{

    void Logger::Init() {
        // AI on
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            auto logger = std::make_shared<spdlog::logger>("main", console_sink);
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::debug);

            spdlog::info("Logger initialized successfully");
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
        // AI off
    }

}