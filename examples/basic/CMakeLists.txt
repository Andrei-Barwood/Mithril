# examples/basic/CMakeLists.txt

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

include(FetchContent)

set(MITHRIL_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(MITHRIL_V2_ARCHIVE "${MITHRIL_REPO_ROOT}/Mithril_API_V2/build-cmake/libmithril_v2.a")

find_path(MITHRIL_ASIO_INCLUDE_DIR asio.hpp)
if(NOT MITHRIL_ASIO_INCLUDE_DIR)
  message(STATUS "Standalone Asio not found in system include paths; fetching from upstream.")
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
target_compile_definitions(mithril_boost_asio_compat PUBLIC ASIO_STANDALONE)
target_include_directories(mithril_boost_asio_compat
  PUBLIC
    "${MITHRIL_REPO_ROOT}/network/include"
    "${MITHRIL_ASIO_INCLUDE_DIR}"
)

function(mithril_configure_example target_name source_file)
  add_executable(${target_name} ${source_file})
  target_compile_features(${target_name} PRIVATE cxx_std_20)
  target_compile_definitions(${target_name} PRIVATE ASIO_STANDALONE)
  target_include_directories(${target_name}
    PRIVATE
      "${MITHRIL_REPO_ROOT}/network/include"
      "${MITHRIL_REPO_ROOT}/Mithril_API_V2/include"
      "${MITHRIL_ASIO_INCLUDE_DIR}"
      ${SODIUM_INCLUDE_DIRS}
  )

  if(TARGET mithril_v2)
    target_link_libraries(${target_name}
      PRIVATE
        mithril_boost_asio_compat
        mithril_v2
        Threads::Threads
        PkgConfig::SODIUM
    )
  elseif(EXISTS "${MITHRIL_V2_ARCHIVE}")
    target_link_libraries(${target_name}
      PRIVATE
        mithril_boost_asio_compat
        ${MITHRIL_V2_ARCHIVE}
        Threads::Threads
        PkgConfig::SODIUM
    )
  else()
    message(FATAL_ERROR
      "mithril_v2 target not found and archive missing at ${MITHRIL_V2_ARCHIVE}. "
      "Build Mithril_API_V2 first or add it to this CMake project.")
  endif()

  if(APPLE)
    target_link_libraries(${target_name} PRIVATE "-framework Security")
  endif()
endfunction()

mithril_configure_example(accepting_connection_sodium accepting_connection_sodium.cpp)
mithril_configure_example(secure_client secure_client.cpp)
