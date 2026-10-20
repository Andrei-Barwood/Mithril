# ============================================================================
# Mithril - Root CMake Build System
# File: CMakeLists.txt
# Version: 2.0.0
# ============================================================================

cmake_minimum_required(VERSION 3.20)

project(Mithril
    VERSION 2.0.0
    DESCRIPTION "High-performance cryptographic security layer for renewable energy IoT & critical infrastructure"
    LANGUAGES C CXX
)

# ----------------------------------------------------------------------------
# CMake Module Path & Helper Modules
# ----------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

include(CompilerWarnings)
include(Sanitizers)

# ----------------------------------------------------------------------------
# Standards
# ----------------------------------------------------------------------------
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# Global Configuration Options
# ----------------------------------------------------------------------------
option(MITHRIL_BUILD_TESTS "Build Mithril test suites" ON)
option(MITHRIL_BUILD_EXAMPLES "Build Mithril network examples" ON)

# Provider options for Mithril_API_V2
option(MITHRIL_USE_LIBSODIUM "Enable libsodium-backed crypto provider" ON)
option(MITHRIL_ENABLE_PROVIDER_FLINT "Enable FLINT provider registration path" OFF)
option(MITHRIL_USE_FLINT "Enable FLINT-backed bigint/modarith implementation when available" ON)

# Warnings & Sanitizers
option(MITHRIL_ENABLE_WARNINGS "Enable standard compiler warnings" ON)
option(MITHRIL_ENABLE_WERROR "Treat warnings as errors" OFF)
option(MITHRIL_ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer (ASan)" OFF)
option(MITHRIL_ENABLE_SANITIZER_UNDEFINED "Enable UndefinedBehaviorSanitizer (UBSan)" OFF)
option(MITHRIL_ENABLE_SANITIZER_THREAD "Enable ThreadSanitizer (TSan)" OFF)

# Propagate options to subprojects
set(MITHRIL_ENABLE_TESTS ${MITHRIL_BUILD_TESTS} CACHE BOOL "Propagate test toggle to Mithril_API_V2" FORCE)

# ----------------------------------------------------------------------------
# Testing
# ----------------------------------------------------------------------------
if(MITHRIL_BUILD_TESTS)
    enable_testing()
endif()

# ----------------------------------------------------------------------------
# Subdirectories
# ----------------------------------------------------------------------------
message(STATUS "Configuring Mithril Core (Mithril_API_V2)...")
add_subdirectory(Mithril_API_V2)

message(STATUS "Configuring Mithril Crypto Transition Layer (crypto)...")
add_subdirectory(crypto)

message(STATUS "Configuring Mithril Network Layer (network)...")
add_subdirectory(network)

if(MITHRIL_BUILD_EXAMPLES)
    message(STATUS "Configuring Mithril Consumer Examples (examples)...")
    add_subdirectory(examples)
endif()

# Apply compiler warnings to targets if enabled
if(MITHRIL_ENABLE_WARNINGS)
    if(TARGET mithril_v2)
        mithril_set_compiler_warnings(mithril_v2 IS_C_ONLY TRUE ENABLE_WERROR ${MITHRIL_ENABLE_WERROR})
        mithril_enable_sanitizers(mithril_v2)
    endif()

    if(TARGET mithril_crypto)
        mithril_set_compiler_warnings(mithril_crypto IS_C_ONLY TRUE ENABLE_WERROR ${MITHRIL_ENABLE_WERROR})
        mithril_enable_sanitizers(mithril_crypto)
    endif()

    if(TARGET mithril_network)
        mithril_set_compiler_warnings(mithril_network IS_C_ONLY FALSE ENABLE_WERROR ${MITHRIL_ENABLE_WERROR})
        mithril_enable_sanitizers(mithril_network)
    endif()
endif()

# ----------------------------------------------------------------------------
# Build Status Summary
# ----------------------------------------------------------------------------
message(STATUS "==========================================================")
message(STATUS " Mithril Configuration Summary")
message(STATUS "==========================================================")
message(STATUS " Version:                 ${PROJECT_VERSION}")
message(STATUS " C Standard:              C${CMAKE_C_STANDARD}")
message(STATUS " C++ Standard:            C++${CMAKE_CXX_STANDARD}")
message(STATUS " Build Tests:             ${MITHRIL_BUILD_TESTS}")
message(STATUS " Build Examples:          ${MITHRIL_BUILD_EXAMPLES}")
message(STATUS " Use libsodium:           ${MITHRIL_USE_LIBSODIUM}")
message(STATUS " Enable FLINT provider:   ${MITHRIL_ENABLE_PROVIDER_FLINT}")
message(STATUS " Compiler Warnings:       ${MITHRIL_ENABLE_WARNINGS}")
message(STATUS " Warnings as Errors:      ${MITHRIL_ENABLE_WERROR}")
message(STATUS " AddressSanitizer:        ${MITHRIL_ENABLE_SANITIZER_ADDRESS}")
message(STATUS " UndefinedSanitizer:      ${MITHRIL_ENABLE_SANITIZER_UNDEFINED}")
message(STATUS " ThreadSanitizer:         ${MITHRIL_ENABLE_SANITIZER_THREAD}")
message(STATUS "==========================================================")
