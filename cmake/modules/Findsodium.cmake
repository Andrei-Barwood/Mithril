# ============================================================================
# Mithril - Find Module for libsodium
# File: cmake/modules/Findsodium.cmake
# ============================================================================

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SODIUM QUIET libsodium)
endif()

find_path(SODIUM_INCLUDE_DIR
    NAMES sodium.h
    HINTS
        ${PC_SODIUM_INCLUDE_DIRS}
        /opt/homebrew/include
        /usr/local/include
        /usr/include
    PATH_SUFFIXES sodium
)

find_library(SODIUM_LIBRARY
    NAMES sodium libsodium
    HINTS
        ${PC_SODIUM_LIBRARY_DIRS}
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sodium
    REQUIRED_VARS SODIUM_LIBRARY SODIUM_INCLUDE_DIR
    VERSION_VAR PC_SODIUM_VERSION
)

if(sodium_FOUND AND NOT TARGET sodium)
    add_library(sodium UNKNOWN IMPORTED)
    set_target_properties(sodium PROPERTIES
        IMPORTED_LOCATION "${SODIUM_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(SODIUM_INCLUDE_DIR SODIUM_LIBRARY)
