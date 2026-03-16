# examples/basic/CMakeLists.txt

find_package(Boost 1.82 REQUIRED COMPONENTS system)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

set(MITHRIL_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(MITHRIL_V2_ARCHIVE "${MITHRIL_REPO_ROOT}/Mithril_API_V2/build-cmake/libmithril_v2.a")

function(mithril_configure_example target_name source_file)
  add_executable(${target_name} ${source_file})
  target_compile_features(${target_name} PRIVATE cxx_std_20)
  target_include_directories(${target_name}
    PRIVATE
      "${MITHRIL_REPO_ROOT}/network/include"
      "${MITHRIL_REPO_ROOT}/Mithril_API_V2/include"
      ${SODIUM_INCLUDE_DIRS}
  )

  if(TARGET mithril_v2)
    target_link_libraries(${target_name}
      PRIVATE
        Boost::system
        mithril_v2
        ${SODIUM_LIBRARIES}
    )
  elseif(EXISTS "${MITHRIL_V2_ARCHIVE}")
    target_link_libraries(${target_name}
      PRIVATE
        Boost::system
        ${MITHRIL_V2_ARCHIVE}
        ${SODIUM_LIBRARIES}
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
