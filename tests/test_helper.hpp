// ============================================================================
// Mithril Test Suite - Test Helper & Assertion Framework
// ============================================================================
// File: tests/test_helper.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_TEST_HELPER_HPP
#define MITHRIL_TEST_HELPER_HPP

#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace mithril::test {

inline int &total_assertions() {
    static int count = 0;
    return count;
}

inline int &failed_assertions() {
    static int count = 0;
    return count;
}

inline void assert_impl(bool condition, const char *expr, const char *file, int line, const std::string &msg = "") {
    ++total_assertions();
    if (!condition) {
        ++failed_assertions();
        std::cerr << "  [ASSERTION FAILED] " << file << ":" << line << "\n"
                  << "    Expression: " << expr << "\n";
        if (!msg.empty()) {
            std::cerr << "    Message:    " << msg << "\n";
        }
    }
}

#define MITHRIL_TEST_ASSERT(cond) \
    ::mithril::test::assert_impl((cond), #cond, __FILE__, __LINE__)

#define MITHRIL_TEST_ASSERT_MSG(cond, msg) \
    ::mithril::test::assert_impl((cond), #cond, __FILE__, __LINE__, (msg))

#define MITHRIL_TEST_ASSERT_EQ(a, b) \
    ::mithril::test::assert_impl(((a) == (b)), #a " == " #b, __FILE__, __LINE__)

#define MITHRIL_TEST_ASSERT_NE(a, b) \
    ::mithril::test::assert_impl(((a) != (b)), #a " != " #b, __FILE__, __LINE__)

#define MITHRIL_TEST_ASSERT_THROWS(expr, ExceptionType)               \
    do {                                                              \
        bool caught = false;                                          \
        try {                                                         \
            expr;                                                     \
        } catch (const ExceptionType &) {                             \
            caught = true;                                            \
        } catch (...) {                                               \
            caught = false;                                           \
        }                                                             \
        ::mithril::test::assert_impl(caught, #expr " throws " #ExceptionType, __FILE__, __LINE__); \
    } while (0)

inline int report_and_exit(std::string_view test_suite_name) {
    std::cout << "\n------------------------------------------------------------\n";
    std::cout << "Test Suite: " << test_suite_name << "\n";
    std::cout << "Assertions: " << total_assertions() << " total, "
              << failed_assertions() << " failed.\n";
    if (failed_assertions() == 0) {
        std::cout << "Result:     [PASSED] ✅\n";
        std::cout << "------------------------------------------------------------\n";
        return 0;
    } else {
        std::cerr << "Result:     [FAILED] ❌ (" << failed_assertions() << " failures)\n";
        std::cout << "------------------------------------------------------------\n";
        return 1;
    }
}

inline std::vector<uint8_t> make_pattern(size_t len, uint8_t seed = 0x5A) {
    std::vector<uint8_t> buf(len);
    for (size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<uint8_t>((seed + i * 31u) & 0xFFu);
    }
    return buf;
}

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }

    double elapsed_ms() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

    double elapsed_us() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(end - start_).count();
    }

    double elapsed_sec() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace mithril::test

#endif // MITHRIL_TEST_HELPER_HPP
