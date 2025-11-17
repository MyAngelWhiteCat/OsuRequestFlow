#include "../../src/file_manager.h"
#include <filesystem>
#include <chrono>
#include <thread>

namespace test_file_manager {

    void TestWrite(file_manager::FileManager& fm) {
        fm.WriteInRoot({ 'q', 'w', 'r' }, "test.txt");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "zest.txw");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "tzst.txq");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "test.trt");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "teut.tzt");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "tent.tct");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "tezt.tbt");
        fm.WriteInRoot({ 'q', 'w', 'r' }, "teqt.tyt");

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