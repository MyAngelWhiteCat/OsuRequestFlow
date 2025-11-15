#include "../../src/file_manager.h"
#include <filesystem>
#include <chrono>
#include <thread>

namespace test_file_manager {

    void TestWrite(file_manager::FileManager& fm) {
        fm.WriteInRoot("Qello", "test.txt");
        fm.WriteInRoot("Hello", "zest.txw");
        fm.WriteInRoot("Hello", "tzst.txq");
        fm.WriteInRoot("Hello", "test.trt");
        fm.WriteInRoot("Hello", "teut.tzt");
        fm.WriteInRoot("Hello", "tent.tct");
        fm.WriteInRoot("Hello", "tezt.tbt");
        fm.WriteInRoot("Hello", "teqt.tyt");

    }

    void TestDelete(file_manager::FileManager& fm) {
        fm.DeleteAllWritedFilesFromHistory();
    }

    void RunTests() {
        auto path = std::filesystem::path(std::filesystem::current_path().string() + "/test");
        file_manager::FileManager fm(path);
        TestWrite(fm);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        TestDelete(fm);
    }

}