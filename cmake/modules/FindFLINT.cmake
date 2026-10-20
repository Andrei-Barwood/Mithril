# ============================================================================
# Mithril - Find Module for FLINT (Fast Library for Number Theory)
# File: cmake/modules/FindFLINT.cmake
# ============================================================================

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_FLINT QUIET flint)
endif()

find_path(FLINT_INCLUDE_DIR
    NAMES flint/flint.h flint.h
    HINTS
        ${PC_FLINT_INCLUDE_DIRS}
        /opt/homebrew/include
        /usr/local/include
        /usr/include
    PATH_SUFFIXES flint
)

find_library(FLINT_LIBRARY
    NAMES flint libflint
    HINTS
        ${PC_FLINT_LIBRARY_DIRS}
        /opt/homebrew/lib
        /usr/local/lib
        /usr/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FLINT
    REQUIRED_VARS FLINT_LIBRARY FLINT_INCLUDE_DIR
    VERSION_VAR PC_FLINT_VERSION
)

if(FLINT_FOUND AND NOT TARGET flint)
    add_library(flint UNKNOWN IMPORTED)
    set_target_properties(flint PROPERTIES
        IMPORTED_LOCATION "${FLINT_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FLINT_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(FLINT_INCLUDE_DIR FLINT_LIBRARY)
