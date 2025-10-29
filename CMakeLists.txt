cmake_minimum_required(VERSION 3.11)
project(orf CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_WIN32_WINNT=0x0601)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

find_package(Boost 1.84.0 REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(orf src/main.cpp
                   src/irc.h
                   src/irc.cpp
                   src/domain.h)

target_include_directories(orf PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(orf PRIVATE ${openssl_PACKAGE_FOLDER_RELEASE}/include)

target_link_libraries(orf PRIVATE 
    ${Boost_LIBRARIES}
    ${openssl_PACKAGE_FOLDER_RELEASE}/lib/libssl.lib
    ${openssl_PACKAGE_FOLDER_RELEASE}/lib/libcrypto.lib
    ${zlib_PACKAGE_FOLDER_RELEASE}/lib/zlib.lib
    ws2_32 
    crypt32
)