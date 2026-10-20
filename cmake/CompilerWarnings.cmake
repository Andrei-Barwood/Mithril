# ============================================================================
# Mithril - Compiler Warnings Configuration
# File: cmake/CompilerWarnings.cmake
# ============================================================================

function(mithril_set_compiler_warnings target_name)
    set(oneValueArgs IS_C_ONLY ENABLE_WERROR)
    cmake_parse_arguments(WARN "" "${oneValueArgs}" "" ${ARGN})

    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )

    set(CLANG_C_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wcast-align
        -Wunused
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wstrict-prototypes
        -Wmissing-prototypes
    )

    set(GCC_C_WARNINGS
        ${CLANG_C_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
    )

    set(MSVC_WARNINGS
        /W4
        /permissive-
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
    )

    if(WARN_ENABLE_WERROR)
        list(APPEND CLANG_WARNINGS -Werror)
        list(APPEND GCC_WARNINGS -Werror)
        list(APPEND CLANG_C_WARNINGS -Werror)
        list(APPEND GCC_C_WARNINGS -Werror)
        list(APPEND MSVC_WARNINGS /WX)
    endif()

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_C_COMPILER_ID MATCHES ".*Clang")
        if(WARN_IS_C_ONLY)
            target_compile_options(${target_name} PRIVATE ${CLANG_C_WARNINGS})
        else()
            target_compile_options(${target_name} PRIVATE ${CLANG_WARNINGS})
        endif()
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        if(WARN_IS_C_ONLY)
            target_compile_options(${target_name} PRIVATE ${GCC_C_WARNINGS})
        else()
            target_compile_options(${target_name} PRIVATE ${GCC_WARNINGS})
        endif()
    endif()
endfunction()
