# ============================================================================
# Mithril Crypto Layer - CMake Configuration
# File: crypto/CMakeLists.txt
# ============================================================================

add_library(mithril_crypto STATIC
    src/mithril_crypto.c
    src/flint_wrapper.c
)

target_include_directories(mithril_crypto
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(mithril_crypto
    PUBLIC
        mithril_v2
)
