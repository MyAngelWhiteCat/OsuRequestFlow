#include "logging.h"
//AI on
namespace logging {

    void Logger::Init() {
        try {
            spdlog::init_thread_pool(8192, 1);

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            auto logger = std::make_shared<spdlog::async_logger>(
                "main",
                console_sink,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::debug);
            
            spdlog::info("Logger initialized successfully");
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    void Logger::Shutdown() {
        spdlog::shutdown();
    }
}
// AI off