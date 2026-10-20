# ============================================================================
# Mithril Examples - Basic Network Consumers
# File: examples/basic/CMakeLists.txt
# ============================================================================

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

set(MITHRIL_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")

# If building inside root project, mithril_network / mithril_v2 targets are already defined.
# If building standalone, ensure dependencies and compatibility target.
if(NOT TARGET mithril_v2)
    set(MITHRIL_V2_ARCHIVE "${MITHRIL_REPO_ROOT}/Mithril_API_V2/build-cmake/libmithril_v2.a")
    if(EXISTS "${MITHRIL_V2_ARCHIVE}")
        add_library(mithril_v2 STATIC IMPORTED GLOBAL)
        set_target_properties(mithril_v2 PROPERTIES
            IMPORTED_LOCATION "${MITHRIL_V2_ARCHIVE}"
            INTERFACE_INCLUDE_DIRECTORIES "${MITHRIL_REPO_ROOT}/Mithril_API_V2/include"
        )
    else()
        message(FATAL_ERROR
            "mithril_v2 target not found and archive missing at ${MITHRIL_V2_ARCHIVE}. "
            "Please build from root repository using 'cmake -B build' or build Mithril_API_V2 first.")
    endif()
endif()

if(NOT TARGET mithril_boost_asio_compat AND NOT TARGET mithril_network)
    include(FetchContent)
    find_path(MITHRIL_ASIO_INCLUDE_DIR asio.hpp
        HINTS
            "${MITHRIL_REPO_ROOT}/network/third_party/asio/asio/include"
            /opt/homebrew/include
            /usr/local/include
    )
    if(NOT MITHRIL_ASIO_INCLUDE_DIR)
        FetchContent_Declare(
            asio
            URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(asio)
        set(MITHRIL_ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include")
    endif()

    add_library(mithril_boost_asio_compat STATIC
        "${MITHRIL_REPO_ROOT}/network/src/boost_asio_compat.cpp"
    )
    target_compile_features(mithril_boost_asio_compat PUBLIC cxx_std_20)
    target_compile_definitions(mithril_boost_asio_compat PUBLIC ASIO_STANDALONE MITHRIL_CONSUMER_PATH_V2_DEFAULT=1)
    target_include_directories(mithril_boost_asio_compat PUBLIC
        "${MITHRIL_REPO_ROOT}/network/include"
        "${MITHRIL_ASIO_INCLUDE_DIR}"
    )
    target_link_libraries(mithril_boost_asio_compat PUBLIC mithril_v2 Threads::Threads)
endif()

# Determine network target to link
if(TARGET mithril_network)
    set(MITHRIL_NET_TARGET mithril_network)
else()
    set(MITHRIL_NET_TARGET mithril_boost_asio_compat)
endif()

function(mithril_configure_example target_name source_file)
    add_executable(${target_name} ${source_file})
    target_compile_features(${target_name} PRIVATE cxx_std_20)
    target_compile_definitions(${target_name} PRIVATE ASIO_STANDALONE)
    target_include_directories(${target_name}
        PRIVATE
            "${MITHRIL_REPO_ROOT}/network/include"
            "${MITHRIL_REPO_ROOT}/Mithril_API_V2/include"
            ${SODIUM_INCLUDE_DIRS}
    )

    target_link_libraries(${target_name}
        PRIVATE
            ${MITHRIL_NET_TARGET}
            mithril_v2
            Threads::Threads
            PkgConfig::SODIUM
    )

    if(APPLE)
        target_link_libraries(${target_name} PRIVATE "-framework Security")
    endif()
endfunction()

# Configure valid working examples
mithril_configure_example(accepting_connection_sodium accepting_connection_sodium.cpp)
mithril_configure_example(secure_client secure_client.cpp)
mithril_configure_example(binding_socket_sodium binding_socket_sodium.cpp)
mithril_configure_example(test_rollout_toggles test_rollout_toggles.cpp)
mithril_configure_example(test_crypto_telemetry test_crypto_telemetry.cpp)
mithril_configure_example(async_server_sodium async_server_sodium.cpp)
mithril_configure_example(connecting_client_sodium connecting_client_sodium.cpp)
mithril_configure_example(echo_server_sodium echo_server_sodium.cpp)
