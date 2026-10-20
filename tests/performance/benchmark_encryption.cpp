// ============================================================================
// Mithril Tests - Performance: AEAD Encryption Scaling Benchmark
// ============================================================================
// File: tests/performance/benchmark_encryption.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;
using mithril::test::Timer;

int main() {
    std::cout << "[BENCHMARK] AEAD Encryption Scaling across Block Sizes\n";
    std::cout << "============================================================\n";

    AuthenticatedEncryption crypto;

    struct BenchmarkConfig {
        size_t block_size;
        int iterations;
    };

    const std::vector<BenchmarkConfig> configs = {
        {64, 50000},
        {512, 20000},
        {4096, 5000},
        {65536, 1000},
        {1048576, 50}
    };

    for (const auto &cfg : configs) {
        std::vector<uint8_t> data(cfg.block_size, 0x5C);

        Timer timer;
        for (int i = 0; i < cfg.iterations; ++i) {
            auto ct = crypto.encrypt(data);
            (void)ct;
        }
        double sec = timer.elapsed_sec();
        double total_mb = (static_cast<double>(cfg.block_size) * cfg.iterations) / (1024.0 * 1024.0);
        double mb_per_sec = total_mb / sec;

        std::cout << "  Block Size: " << std::setw(8) << cfg.block_size << " B | "
                  << "Rate: " << std::fixed << std::setprecision(2) << std::setw(8) << mb_per_sec << " MB/s | "
                  << "Time: " << std::setprecision(3) << sec << " s\n";

        MITHRIL_TEST_ASSERT(mb_per_sec > 5.0);
    }

    std::cout << "============================================================\n";
    return mithril::test::report_and_exit("Performance: Encryption Scaling");
}
