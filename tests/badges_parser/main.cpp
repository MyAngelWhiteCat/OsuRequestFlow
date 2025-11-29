#include "test_badges_parser.h"

int main() {
#include <iostream>
    // MSVC
#ifdef _MSC_VER
    std::cout << "MSVC version: " << _MSC_VER << std::endl;
    std::cout << "Full version: " << _MSC_FULL_VER << std::endl;
#endif

    // GCC
#ifdef __GNUC__
    std::cout << "GCC version: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
#endif

    // Clang
#ifdef __clang__
    std::cout << "Clang version: " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << std::endl;
#endif

    // C++ standard
    std::cout << "C++ standard: " << __cplusplus << std::endl;
    test_badge_parser::RunTests();

}