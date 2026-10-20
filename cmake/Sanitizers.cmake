# ============================================================================
# Mithril - Sanitizers Configuration
# File: cmake/Sanitizers.cmake
# ============================================================================

function(mithril_enable_sanitizers target_name)
    if(MSVC)
        return()
    endif()

    set(SANITIZERS "")

    if(MITHRIL_ENABLE_SANITIZER_ADDRESS)
        list(APPEND SANITIZERS "address")
    endif()

    if(MITHRIL_ENABLE_SANITIZER_UNDEFINED)
        list(APPEND SANITIZERS "undefined")
    endif()

    if(MITHRIL_ENABLE_SANITIZER_THREAD)
        if("address" IN_LIST SANITIZERS)
            message(WARNING "ThreadSanitizer cannot be combined with AddressSanitizer. Skipping ThreadSanitizer.")
        else()
            list(APPEND SANITIZERS "thread")
        endif()
    endif()

    list(JOIN SANITIZERS "," SANITIZERS_STR)

    if(NOT "${SANITIZERS_STR}" STREQUAL "")
        target_compile_options(${target_name} PRIVATE -fsanitize=${SANITIZERS_STR} -fno-omit-frame-pointer)
        target_link_options(${target_name} PRIVATE -fsanitize=${SANITIZERS_STR})
        message(STATUS "Enabled sanitizers for target '${target_name}': ${SANITIZERS_STR}")
    endif()
endfunction()
