#include <windows.h>
#include <shellapi.h>
#include <string>

int main() {
    std::string url = "http://127.0.0.1:23140";
    HINSTANCE result = ShellExecuteA(
        nullptr,
        "open",
        url.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
}