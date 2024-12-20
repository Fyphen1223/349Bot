# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.16)
# Project name, version and description
project(349Bot VERSION 0.0.1 DESCRIPTION "349Bot is a blazing-fast C++ based Discord music bot.")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(OPENSSL_ROOT_DIR /usr/local/ssl)
set(OPENSSL_INCLUDE_DIR /usr/local/ssl/include)
set(OPENSSL_CRYPTO_LIBRARY /usr/lib/x86_64-linux-gnu/libcrypto.so)
set(OPENSSL_SSL_LIBRARY /usr/lib/x86_64-linux-gnu/libssl.so)
set(OPENSSL_LIBRARIES /usr/local/ssl/lib)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(OPUS REQUIRED opus)

# Create an executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

find_package(DPP CONFIG REQUIRED)

# Link the pre-installed DPP package.
target_link_libraries(${PROJECT_NAME} PRIVATE dpp::dpp ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY} ZLIB::ZLIB ${OPUS_LIBRARIES})

# Include the DPP directories.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIR} ${OPUS_INCLUDE_DIRS}
)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)