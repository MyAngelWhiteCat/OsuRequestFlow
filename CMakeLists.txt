cmake_minimum_required(VERSION 3.11)
project(OsuRequestFlow CXX)

include_directories(src)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_WIN32_WINNT=0x0601)

if(MSVC)
    add_compile_options(/bigobj)
endif()

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_definitions(BOOST_ALL_NO_LIB=1)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(Boost 1.84.0 REQUIRED COMPONENTS system)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

############################################# libraries

add_library(Logger STATIC 
    src/logging.h
    src/logging.cpp
)

target_link_libraries(Logger PUBLIC
    spdlog::spdlog
)

target_include_directories(Logger PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${spdlog_INCLUDE_DIRS}
    ${fmt_INCLUDE_DIRS}
)

########################################## ssl serts loader

add_library(SSLCertsLoader STATIC 
    src/ca_sertificates_loader.h 
    src/ca_sertificates_loader.cpp
)

target_include_directories(SSLCertsLoader PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(SSLCertsLoader PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

if(WIN32)
    target_link_libraries(SSLCertsLoader PUBLIC ws2_32 crypt32 user32)
endif()

########################################## http server

add_library(HTTPServer STATIC 
    src/decode_url.h 
    src/http_server.h 
    src/http_server.cpp 
    src/root_directory.h 
    src/root_directory.cpp
    src/domain_fields.h
    src/tcp_listener.h
)

target_include_directories(HTTPServer PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(HTTPServer PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    ZLIB::ZLIB
    Logger
)


########################################## Connection

add_library(Connection STATIC 
    src/connection.h
    src/connection.cpp
)

target_link_libraries(Connection PUBLIC 
    SSLCertsLoader
    Logger
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

########################################## Chat bot

add_library(ChatBot STATIC 
    src/command.h 
    src/command_executor.h 
    src/command_executor.cpp
    src/command_parser.h
    src/command_parser.cpp
    src/user_validator.h
    src/user_validator.cpp
    src/chat_bot.h 
    src/chat_bot.cpp
)

target_link_libraries(ChatBot PUBLIC
    Logger 
)

########################################## irc client

add_library(IRCClient STATIC
    src/irc_client.h

    src/message.h
    src/message.cpp
    src/message_handler.h
    src/message_processor.h 
    src/message_processor.cpp

    src/domain.h

    src/auth_data.h
    src/auth_data.cpp
)

target_link_libraries(IRCClient PUBLIC 
    Connection
    Logger
)

########################################## http client

add_library(HTTPClient STATIC
    tests/http_client/main.cpp 
    tests/http_client/test_http_client.h

    src/http_client.h 
    src/http_client.cpp

    src/request_builder.h 
    src/request_builder.cpp
    
    src/response_parser.h
    src/response_parser.cpp

    src/random_user_agent.h
    src/random_user_agent.cpp
    
    src/connection_pool.h 
    src/connection_pool.cpp 

    src/decode_url.h
)

target_link_libraries(HTTPClient PUBLIC 
    SSLCertsLoader
    Logger
    nlohmann_json::nlohmann_json
    Boost::system
    Boost::thread
    ZLIB::ZLIB
)

########################################## file manager

add_library(FileManager STATIC 
    src/file_manager.h
    src/file_manager.cpp
)
target_link_libraries(FileManager PUBLIC 
    Logger
)

########################################## downloader

add_library(Downloader STATIC
    src/downloader.h 
    src/downloader.cpp
)

target_link_libraries(Downloader PUBLIC 
    HTTPClient 
    FileManager
)

########################################## WebSocket server

add_library(WebSocketServer STATIC 
    src/websoc_server.h 
)

target_include_directories(WebSocketServer PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    Logger
)

target_link_libraries(WebSocketServer PUBLIC
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    ZLIB::ZLIB
    Logger
)

########################################## ChatWidget

add_library(ChatWidget STATIC
    src/chat_widget.h
    src/chat_widget.cpp
)

target_link_libraries(ChatWidget PUBLIC 
    WebSocketServer
)

########################################## core

add_library(Core STATIC
    src/core.h 
    src/core.cpp 
)

target_link_libraries(Core PUBLIC 
    IRCClient
    Downloader
    ChatBot
    nlohmann_json::nlohmann_json
)

############################################# tests

add_executable(TestTwitchIRCClient 
    tests/twitch_irc_client/main.cpp
)

target_link_libraries(TestTwitchIRCClient PUBLIC
    IRCClient
    Downloader
    ChatBot
)

##########################################

add_executable(TestDownloader
    tests/downloader/test_downloader.h 
    tests/downloader/main.cpp
)

target_link_libraries(TestDownloader PUBLIC 
    Downloader 
)

##########################################

add_executable(TestFileManager
    tests/file_manager/test_file_manager.h 
    tests/file_manager/main.cpp 
)

target_link_libraries(TestFileManager PUBLIC 
    FileManager
)

##########################################

add_executable(TestHttpClient
    tests/http_client/main.cpp 
    tests/http_client/test_http_client.h
)

target_link_libraries(TestHttpClient PUBLIC
    HTTPClient 
)

##########################################

add_executable(TestMessageProcessor
    tests/message_processor/main.cpp 
    tests/message_processor/test_message_processor.h 
    tests/utils.h 
)

target_link_libraries(TestMessageProcessor PUBLIC 
    IRCClient
)

##########################################

add_executable(TestBadgeParser
    tests/badges_parser/main.cpp 
    tests/badges_parser/test_badges_parser.h
)

target_link_libraries(TestBadgeParser PUBLIC 
    IRCClient
)

##########################################

add_executable(TestCommandParser
    tests/command_parser/main.cpp 
    tests/command_parser/test_command_parser.h
)

target_link_libraries(TestCommandParser PUBLIC
    IRCClient
)

##########################################

add_executable(TestUserVerificator 
    tests/verificator/main.cpp 
)
target_link_libraries(TestUserVerificator PUBLIC
    IRCClient 
    FileManager
)

########################################### Main

add_executable(GUI
    src/response_maker.h
    src/response_maker.cpp
    src/request_domain_names.h
    src/request_handler.h 
    src/request_validator.h 
    src/request_validator.cpp 
    src/api_request_handler.h 
    src/file_request_handler.h 
    src/main.cpp
)

target_link_libraries(GUI PUBLIC 
    Core 
    HTTPServer 
)
