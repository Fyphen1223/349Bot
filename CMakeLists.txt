# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.16)
# Project name, version and description
project(349Bot VERSION 0.0.1 DESCRIPTION "349Bot is a blazingly-fast C++ based Discord music bot.")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(OPENSSL_ROOT_DIR /usr/local/ssl)
set(OPENSSL_INCLUDE_DIR /usr/local/ssl/include)
set(OPENSSL_CRYPTO_LIBRARY /usr/lib/x86_64-linux-gnu/libcrypto.so)
set(OPENSSL_SSL_LIBRARY /usr/lib/x86_64-linux-gnu/libssl.so)
set(OPENSSL_LIBRARIES /usr/local/ssl/lib)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

pkg_search_module(OPUS REQUIRED opus)

#include_directories(websocketpp)

# Create an executable
include_directories(/usr/local/include/hv)
link_directories(/usr/local/lib)

add_executable(${PROJECT_NAME}
    src/main.cpp
	src/lib/print.cpp
	src/lib/log.cpp
	src/util/register.cpp
	src/util/device.cpp
	src/util/decoration.cpp
	src/util/textcommands.cpp
	src/events/messageCreate.cpp
	src/events/slashcommandsCreate.cpp
	src/lavacop/lavacop.cpp
	src/lavacop/src/lavalink.cpp
	src/global.cpp
	src/events/autocomplete.cpp
	src/events/commands.cpp
	src/commands/ping.cpp
	src/commands/play.cpp
)

# add_library(math_utils OBJECT src/lib/math_utils.cppm)


find_package(DPP CONFIG REQUIRED)

# Link the pre-installed DPP package.
target_link_libraries(${PROJECT_NAME} PRIVATE dpp::dpp ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY} ZLIB::ZLIB ${OPUS_LIBRARIES} nlohmann_json::nlohmann_json hv)

# Include the DPP directories.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIR} ${OPUS_INCLUDE_DIRS}
)

# add_dependencies(${PROJECT_NAME} math_utils)

add_compile_options(-fmodules-ts)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
