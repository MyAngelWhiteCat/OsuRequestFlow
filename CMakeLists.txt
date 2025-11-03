cmake_minimum_required(VERSION 3.11)
project(OsuRequestFlow CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_WIN32_WINNT=0x0601)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

find_package(Boost 1.84.0 REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(OsuRequestFlow src/main.cpp
                   src/client.h
                   src/client.cpp
                   src/domain.h
                   src/ca_sertificates_loader.h
                   src/ca_sertificates_loader.cpp
                   src/message.h 
                   src/message.cpp 
                   src/connect.cpp 
                   src/connect.h 
                   src/message_handler.h 
                   src/message_handler.cpp
                   src/auth_data.h 
                   src/auth_data.cpp 

)

target_include_directories(OsuRequestFlow PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(OsuRequestFlow PRIVATE ${openssl_PACKAGE_FOLDER_RELEASE}/include)

target_link_libraries(OsuRequestFlow PRIVATE 
    ${Boost_LIBRARIES}
    ${openssl_PACKAGE_FOLDER_RELEASE}/lib/libssl.lib
    ${openssl_PACKAGE_FOLDER_RELEASE}/lib/libcrypto.lib
    ${zlib_PACKAGE_FOLDER_RELEASE}/lib/zlib.lib
    ws2_32 
    crypt32
)