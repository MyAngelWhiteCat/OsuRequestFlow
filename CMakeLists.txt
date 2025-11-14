cmake_minimum_required(VERSION 3.11)
project(OsuRequestFlow CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_WIN32_WINNT=0x0601)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_definitions(BOOST_ALL_NO_LIB=1)


find_package(Boost 1.84.0 REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(OsuRequestFlow 
    src/main.cpp
    src/client.h
    src/client.cpp
    src/connection.h
    src/message.h
    src/message.cpp
    src/message_handler.h
    src/message_handler.cpp
    src/domain.h
    src/auth_data.h
    src/auth_data.cpp
    src/ca_sertificates_loader.h
    src/ca_sertificates_loader.cpp
    src/message_processor.h 
    src/message_processor.cpp
    src/logging.h
    src/logging.cpp
    src/command_parser.h 
    src/command_parser.cpp
    src/downloader.h
    src/downloader.cpp 
    src/file_manager.h 
    src/file_manager.cpp
    src/http_client.h 
    src/http_client.cpp 
    src/random_user_agent.h
)

add_executable(TestMessageProcessor
    tests/message_processor/main.cpp 
    tests/message_processor/test_message_processor.h 
    tests/utils.h 
    src/message_processor.h 
    src/message_processor.cpp
    src/logging.h
    src/logging.cpp
    src/message.h
    src/message.cpp
    src/domain.h
)

target_include_directories(TestMessageProcessor PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(TestMessageProcessor PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

add_executable(TestBadgeParser
    tests/badges_parser/main.cpp 
    tests/badges_parser/test_badges_parser.h
    src/message.h
    src/message.cpp
    src/domain.h
)

target_link_libraries(TestBadgeParser PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

target_include_directories(TestBadgeParser PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

add_executable(TestCommandParser
    tests/command_parser/main.cpp 
    tests/command_parser/test_command_parser.h
    src/command_parser.h
)

add_executable(TestHttpClient
    tests/http_client/main.cpp 
    tests/http_client/test_http_client.h
    src/http_client.h 
    src/random_user_agent.h
    src/ca_sertificates_loader.h 
    src/ca_sertificates_loader.cpp
    src/connection.h
)

target_link_libraries(TestHttpClient PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

target_include_directories(TestHttpClient PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_include_directories(OsuRequestFlow PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)


target_link_libraries(OsuRequestFlow PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(OsuRequestFlow PRIVATE ws2_32 crypt32 user32)
    target_link_libraries(TestMessageProcessor PRIVATE ws2_32 crypt32 user32)
endif()