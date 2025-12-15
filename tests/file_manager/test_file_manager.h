#include "../../src/osu_file_manager.h"
#include <filesystem>
#include <chrono>
#include <thread>

namespace test_file_manager {

    void RunTests() {
        auto path = std::filesystem::path(std::filesystem::current_path().string() + "/test");
        
    }

}