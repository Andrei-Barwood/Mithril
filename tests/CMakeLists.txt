# ============================================================================
# Mithril Tests - Root Test Suite Configuration
# File: tests/CMakeLists.txt
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(MithrilTests LANGUAGES CXX)

find_package(Threads REQUIRED)

function(mithril_add_test_target target_name source_file is_benchmark)
    add_executable(${target_name} ${source_file})
    target_compile_features(${target_name} PRIVATE cxx_std_20)
    target_link_libraries(${target_name}
        PRIVATE
            mithril_network
            mithril_v2
            Threads::Threads
    )

    if(APPLE)
        target_link_libraries(${target_name} PRIVATE "-framework Security")
    endif()

    if(MITHRIL_ENABLE_WARNINGS)
        mithril_set_compiler_warnings(${target_name} IS_C_ONLY FALSE ENABLE_WERROR ${MITHRIL_ENABLE_WERROR})
    endif()

    # Register with CTest
    if(NOT is_benchmark)
        add_test(NAME ${target_name} COMMAND ${target_name})
        set_tests_properties(${target_name} PROPERTIES TIMEOUT 30)
    else()
        add_test(NAME ${target_name} COMMAND ${target_name})
        set_tests_properties(${target_name} PROPERTIES LABELS "benchmark" TIMEOUT 60)
    endif()
endfunction()

# ----------------------------------------------------------------------------
# 1. Unit Tests
# ----------------------------------------------------------------------------
mithril_add_test_target(test_mithril_sodium unit/test_mithril_sodium.cpp FALSE)
mithril_add_test_target(test_encryption unit/test_encryption.cpp FALSE)
mithril_add_test_target(test_hash unit/test_hash.cpp FALSE)
mithril_add_test_target(test_key_exchange unit/test_key_exchange.cpp FALSE)
mithril_add_test_target(test_signatures unit/test_signatures.cpp FALSE)

# ----------------------------------------------------------------------------
# 2. Integration Tests
# ----------------------------------------------------------------------------
mithril_add_test_target(test_full_handshake integration/test_full_handshake.cpp FALSE)
mithril_add_test_target(test_concurrent_sessions integration/test_concurrent_sessions.cpp FALSE)
mithril_add_test_target(test_error_handling integration/test_error_handling.cpp FALSE)

# ----------------------------------------------------------------------------
# 3. Security Tests
# ----------------------------------------------------------------------------
mithril_add_test_target(test_replay_attack security/test_replay_attack.cpp FALSE)
mithril_add_test_target(test_mitm_protection security/test_mitm_protection.cpp FALSE)
mithril_add_test_target(fuzz_protocol security/fuzz_protocol.cpp FALSE)

# ----------------------------------------------------------------------------
# 4. Performance Benchmarks
# ----------------------------------------------------------------------------
mithril_add_test_target(benchmark_latency performance/benchmark_latency.cpp TRUE)
mithril_add_test_target(benchmark_throughput performance/benchmark_throughput.cpp TRUE)
mithril_add_test_target(benchmark_encryption performance/benchmark_encryption.cpp TRUE)
