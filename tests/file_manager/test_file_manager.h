#include "../../src/file_manager.h"
#include <filesystem>
#include <chrono>
#include <thread>

namespace test_file_manager {

    void TestWrite(osu_file_manager::OsuFileManager& fm) {
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "test.txt");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "zest.txw");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "tzst.txq");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "test.trt");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "teut.tzt");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "tent.tct");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "tezt.tbt");
        fm.WriteBinaryInRoot({ 'q', 'w', 'r' }, "teqt.tyt");

    }

    void TestDelete(osu_file_manager::OsuFileManager& fm) {
        fm.DeleteAllWritedFilesFromHistory();
    }

    void RunTests() {
        auto path = std::filesystem::path(std::filesystem::current_path().string() + "/test");
        osu_file_manager::OsuFileManager fm(path);
        TestWrite(fm);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        TestDelete(fm);
    }

}