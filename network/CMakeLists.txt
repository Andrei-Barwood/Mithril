# ============================================================================
# Mithril Network Layer - CMake Configuration
# File: network/CMakeLists.txt
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(MithrilNetwork LANGUAGES CXX)

find_package(Threads REQUIRED)

option(MITHRIL_CONSUMER_PATH_V2_DEFAULT "Default consumer crypto path to v2" ON)
option(MITHRIL_ENABLE_V1_COMPAT_PATH "Allow selecting legacy v1 consumer path" OFF)

# ----------------------------------------------------------------------------
# Locate or Fetch Standalone Asio
# ----------------------------------------------------------------------------
set(VENDORED_ASIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/asio/include")

find_path(MITHRIL_ASIO_INCLUDE_DIR asio.hpp
    HINTS
        "${VENDORED_ASIO_DIR}"
        /opt/homebrew/include
        /usr/local/include
        /usr/include
)

if(NOT MITHRIL_ASIO_INCLUDE_DIR)
    message(STATUS "Standalone Asio not found in local vendor or system paths; fetching via FetchContent.")
    include(FetchContent)
    FetchContent_Declare(
        asio
        URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(asio)
    set(MITHRIL_ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "Path to Asio headers" FORCE)
    set(MITHRIL_ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" PARENT_SCOPE)
endif()

message(STATUS "Mithril Network: Using Asio headers from: ${MITHRIL_ASIO_INCLUDE_DIR}")

# ----------------------------------------------------------------------------
# Library: mithril_network
# ----------------------------------------------------------------------------
add_library(mithril_network STATIC
    src/boost_asio_compat.cpp
    src/key_exchange.cpp
    src/stream_encryption.cpp
    src/digital_signature.cpp
    src/crypto_hash.cpp
    src/secure_session.cpp
    src/secure_acceptor.cpp
)

add_library(mithril::network ALIAS mithril_network)
# Alias for compatibility with basic examples
add_library(mithril_boost_asio_compat ALIAS mithril_network)

target_compile_features(mithril_network PUBLIC cxx_std_20)
target_compile_definitions(mithril_network PUBLIC ASIO_STANDALONE)

if(MITHRIL_CONSUMER_PATH_V2_DEFAULT)
    target_compile_definitions(mithril_network PUBLIC MITHRIL_CONSUMER_PATH_V2_DEFAULT=1)
else()
    target_compile_definitions(mithril_network PUBLIC MITHRIL_CONSUMER_PATH_V2_DEFAULT=0)
endif()

if(MITHRIL_ENABLE_V1_COMPAT_PATH)
    target_compile_definitions(mithril_network PUBLIC MITHRIL_ENABLE_V1_COMPAT_PATH=1)
else()
    target_compile_definitions(mithril_network PUBLIC MITHRIL_ENABLE_V1_COMPAT_PATH=0)
endif()

target_include_directories(mithril_network
    SYSTEM PUBLIC
        $<BUILD_INTERFACE:${MITHRIL_ASIO_INCLUDE_DIR}>
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(mithril_network
    PUBLIC
        mithril_v2
        Threads::Threads
)

if(APPLE)
    target_link_libraries(mithril_network PUBLIC "-framework Security")
endif()
