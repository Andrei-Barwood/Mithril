cmake_minimum_required(VERSION 3.20)

project(MithrilAPIv2 VERSION 2.0.0 LANGUAGES C)

option(MITHRIL_ENABLE_TESTS "Build Mithril API v2 tests" ON)
option(MITHRIL_ENABLE_PROVIDER_FLINT "Enable FLINT provider registration path" OFF)
option(MITHRIL_USE_LIBSODIUM "Enable libsodium-backed crypto provider" ON)
option(MITHRIL_USE_FLINT "Enable FLINT-backed bigint/modarith implementation when available" ON)
set(
  MITHRIL_CONFORMANCE_CASES_PER_OPERATION
  "1200"
  CACHE STRING
  "Number of randomized conformance cases executed per operation in test_conformance_randomized.")

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(mithril_v2 STATIC
  src/api/mithril_api.c
  src/api/mithril_context.c
  src/api/mithril_error.c
  src/api/mithril_version.c
  src/api/mithril_rng.c
  src/api/mithril_hash.c
  src/api/mithril_aead.c
  src/api/mithril_kex.c
  src/api/mithril_sign.c
  src/api/mithril_bigint.c
  src/api/mithril_modarith.c
  src/compat/mithril_v1_compat.c
  src/core/core_config.c
  src/core/core_capabilities.c
  src/core/core_validation.c
  src/core/core_status.c
  src/core/core_registry.c
  src/core/core_dispatch.c
  src/core/core_memory.c
  src/core/core_ct.c
  src/core/core_util.c
  src/providers/provider_sodium/provider_sodium_registry.c
  src/providers/provider_sodium/provider_sodium_rng.c
  src/providers/provider_sodium/provider_sodium_hash.c
  src/providers/provider_sodium/provider_sodium_aead.c
  src/providers/provider_sodium/provider_sodium_kex.c
  src/providers/provider_sodium/provider_sodium_sign.c
  src/providers/provider_c23/provider_c23_registry.c
  src/providers/provider_c23/provider_c23_bigint.c
  src/providers/provider_c23/provider_c23_modarith.c
  src/providers/provider_flint/provider_flint_registry.c
  src/providers/provider_flint/provider_flint_bigint.c
  src/providers/provider_flint/provider_flint_modarith.c
  src/platform/apple/apple_entropy.c
  src/platform/apple/apple_clock.c
  src/platform/apple/apple_memory.c
)

target_include_directories(mithril_v2
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MITHRIL_ENABLE_PROVIDER_FLINT)
  target_compile_definitions(mithril_v2 PUBLIC MITHRIL_ENABLE_PROVIDER_FLINT)
endif()

if(MITHRIL_USE_LIBSODIUM)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBSODIUM REQUIRED IMPORTED_TARGET libsodium)

  target_compile_definitions(mithril_v2 PUBLIC MITHRIL_USE_LIBSODIUM)
  target_link_libraries(mithril_v2 PUBLIC PkgConfig::LIBSODIUM)
endif()

set(MITHRIL_FLINT_BACKEND_ENABLED OFF)
if(MITHRIL_ENABLE_PROVIDER_FLINT)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(FLINT QUIET flint)

  if(FLINT_FOUND)
    set(MITHRIL_FLINT_LINK_LIBS ${FLINT_LIBRARIES})
    list(FIND MITHRIL_FLINT_LINK_LIBS flint MITHRIL_FLINT_LINK_LIBS_HAS_FLINT)
    if(MITHRIL_FLINT_LINK_LIBS_HAS_FLINT EQUAL -1)
      list(PREPEND MITHRIL_FLINT_LINK_LIBS flint)
    endif()

    if(MITHRIL_USE_FLINT)
      target_compile_definitions(mithril_v2 PUBLIC MITHRIL_USE_FLINT)
      target_include_directories(mithril_v2 PUBLIC ${FLINT_INCLUDE_DIRS})
      target_link_directories(mithril_v2 PUBLIC ${FLINT_LIBRARY_DIRS})
      target_link_libraries(mithril_v2 PUBLIC ${MITHRIL_FLINT_LINK_LIBS})
      if(FLINT_LDFLAGS_OTHER)
        target_link_options(mithril_v2 PUBLIC ${FLINT_LDFLAGS_OTHER})
      endif()
      set(MITHRIL_FLINT_BACKEND_ENABLED ON)
      message(STATUS "FLINT backend enabled")
    else()
      message(STATUS "FLINT found, backend explicitly disabled")
    endif()
  else()
    if(MITHRIL_USE_FLINT)
      message(FATAL_ERROR "FLINT requested but not found. Install FLINT or configure with -DMITHRIL_USE_FLINT=OFF.")
    else()
      message(STATUS "FLINT not found; backend disabled")
    endif()
  endif()
endif()

if(APPLE)
  target_link_libraries(mithril_v2 PUBLIC "-framework Security")
endif()

if(MITHRIL_ENABLE_TESTS)
  if(NOT MITHRIL_CONFORMANCE_CASES_PER_OPERATION MATCHES "^[1-9][0-9]*$")
    message(FATAL_ERROR "MITHRIL_CONFORMANCE_CASES_PER_OPERATION must be a positive integer.")
  endif()

  enable_testing()

  add_executable(test_c23_bigint tests/unit/test_c23_bigint.c)
  target_link_libraries(test_c23_bigint PRIVATE mithril_v2)

  add_executable(test_c23_modarith tests/unit/test_c23_modarith.c)
  target_link_libraries(test_c23_modarith PRIVATE mithril_v2)

  add_executable(test_provider_activation_failure tests/integration/test_provider_activation_failure.c)
  target_link_libraries(test_provider_activation_failure PRIVATE mithril_v2)
  add_executable(test_sodium_rng_hash tests/unit/test_sodium_rng_hash.c)
  target_link_libraries(test_sodium_rng_hash PRIVATE mithril_v2)
  add_executable(test_sodium_aead_kex tests/integration/test_sodium_aead_kex.c)
  target_link_libraries(test_sodium_aead_kex PRIVATE mithril_v2)
  add_executable(test_sodium_sign tests/integration/test_sodium_sign.c)
  target_link_libraries(test_sodium_sign PRIVATE mithril_v2)
  add_executable(test_error_contracts tests/integration/test_error_contracts.c)
  target_link_libraries(test_error_contracts PRIVATE mithril_v2)
  add_executable(test_v1_compat_smoke tests/integration/test_v1_compat_smoke.c)
  target_link_libraries(test_v1_compat_smoke PRIVATE mithril_v2)
  add_executable(test_v1_compat_legacy tests/integration/test_v1_compat_legacy.c)
  target_link_libraries(test_v1_compat_legacy PRIVATE mithril_v2)
  add_executable(test_v1_compat_deferred_flint tests/integration/test_v1_compat_deferred_flint.c)
  target_link_libraries(test_v1_compat_deferred_flint PRIVATE mithril_v2)

  add_test(NAME c23_bigint COMMAND test_c23_bigint)
  add_test(NAME c23_modarith COMMAND test_c23_modarith)
  add_test(NAME provider_activation_failure COMMAND test_provider_activation_failure)
  add_test(NAME sodium_rng_hash COMMAND test_sodium_rng_hash)
  add_test(NAME sodium_aead_kex COMMAND test_sodium_aead_kex)
  add_test(NAME sodium_sign COMMAND test_sodium_sign)
  add_test(NAME error_contracts COMMAND test_error_contracts)
  add_test(NAME v1_compat_smoke COMMAND test_v1_compat_smoke)
  add_test(NAME v1_compat_legacy COMMAND test_v1_compat_legacy)
  add_test(NAME v1_compat_deferred_flint COMMAND test_v1_compat_deferred_flint)

  if(MITHRIL_FLINT_BACKEND_ENABLED)
    add_executable(test_flint_bigint tests/unit/test_flint_bigint.c)
    target_link_libraries(test_flint_bigint PRIVATE mithril_v2)

    add_executable(test_flint_modarith tests/unit/test_flint_modarith.c)
    target_link_libraries(test_flint_modarith PRIVATE mithril_v2)

    add_executable(test_provider_switch tests/integration/test_provider_switch.c)
    target_link_libraries(test_provider_switch PRIVATE mithril_v2)
    add_executable(test_provider_switch_stress tests/integration/test_provider_switch_stress.c)
    target_link_libraries(test_provider_switch_stress PRIVATE mithril_v2)
    add_executable(test_output_bounds tests/integration/test_output_bounds.c)
    target_link_libraries(test_output_bounds PRIVATE mithril_v2)
    add_executable(test_conformance_vectors tests/integration/test_conformance_vectors.c)
    target_link_libraries(test_conformance_vectors PRIVATE mithril_v2)
    add_executable(test_conformance_randomized tests/integration/test_conformance_randomized.c)
    target_link_libraries(test_conformance_randomized PRIVATE mithril_v2)
    target_compile_definitions(
      test_conformance_randomized
      PRIVATE
        MITHRIL_CONFORMANCE_CASES_PER_OPERATION=${MITHRIL_CONFORMANCE_CASES_PER_OPERATION})

    add_test(NAME flint_bigint COMMAND test_flint_bigint)
    add_test(NAME flint_modarith COMMAND test_flint_modarith)
    add_test(NAME provider_switch COMMAND test_provider_switch)
    add_test(NAME provider_switch_stress COMMAND test_provider_switch_stress)
    add_test(NAME output_bounds COMMAND test_output_bounds)
    add_test(NAME conformance_vectors COMMAND test_conformance_vectors)
    add_test(NAME conformance_randomized COMMAND test_conformance_randomized)

    # On Linux linkers with --as-needed can drop transitive FLINT dependencies
    # from the static core library. Link FLINT explicitly on test executables.
    foreach(mithril_test_target
      test_c23_bigint
      test_c23_modarith
      test_provider_activation_failure
      test_sodium_rng_hash
      test_sodium_aead_kex
      test_sodium_sign
      test_error_contracts
      test_v1_compat_smoke
      test_v1_compat_legacy
      test_v1_compat_deferred_flint
      test_flint_bigint
      test_flint_modarith
      test_provider_switch
      test_provider_switch_stress
      test_output_bounds
      test_conformance_vectors
      test_conformance_randomized)
      if(TARGET ${mithril_test_target})
        target_link_libraries(${mithril_test_target} PRIVATE ${MITHRIL_FLINT_LINK_LIBS})
      endif()
    endforeach()
  endif()
endif()
