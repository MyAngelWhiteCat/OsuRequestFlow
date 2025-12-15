#include "logging.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <iostream>

//AI on
namespace logging {

    void Logger::Init() {
        try {
            spdlog::init_thread_pool(8192, 1);

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("GeneralLogs.txt", true);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };

            auto logger = std::make_shared<spdlog::async_logger>(
                "main",
                sinks.begin(), sinks.end(), 
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