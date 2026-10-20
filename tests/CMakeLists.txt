# ============================================================================
# Mithril Tests - Root Test Suite Configuration
# File: tests/CMakeLists.txt
# ============================================================================

cmake_minimum_required(VERSION 3.20)
project(MithrilTests LANGUAGES CXX)

if(TARGET mithril_network)
    # Unit test: mithril_sodium C++ wrapper
    add_executable(test_mithril_sodium unit/test_mithril_sodium.cpp)
    target_compile_features(test_mithril_sodium PRIVATE cxx_std_20)
    target_link_libraries(test_mithril_sodium PRIVATE mithril_network mithril_v2)
    add_test(NAME test_mithril_sodium COMMAND test_mithril_sodium)
endif()
