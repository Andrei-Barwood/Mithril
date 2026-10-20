// ============================================================================
// Mithril Tests - Performance: Bulk Data Throughput Benchmark
// ============================================================================
// File: tests/performance/benchmark_throughput.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;
using mithril::test::Timer;

int main() {
    std::cout << "[BENCHMARK] Bulk Streaming Data Throughput\n";
    std::cout << "============================================================\n";

    const size_t total_bytes = 10 * 1024 * 1024; // 10 Megabytes
    const size_t chunk_size = 64 * 1024;          // 64 Kilobytes per chunk
    const size_t num_chunks = total_bytes / chunk_size;

    std::vector<uint8_t> chunk_data(chunk_size, 0xAB);
    StreamEncryption sender;
    StreamEncryption receiver(sender.get_key());
    receiver.init_pull_state(sender.get_header());

    std::vector<std::vector<uint8_t>> encrypted_chunks;
    encrypted_chunks.reserve(num_chunks);

    // 1. Encryption Throughput
    Timer enc_timer;
    for (size_t i = 0; i < num_chunks; ++i) {
        bool is_final = (i == num_chunks - 1);
        encrypted_chunks.push_back(sender.encrypt_chunk(chunk_data, is_final));
    }
    double enc_sec = enc_timer.elapsed_sec();
    double enc_mb = static_cast<double>(total_bytes) / (1024.0 * 1024.0);
    double enc_rate = enc_mb / enc_sec;

    std::cout << "  Streaming Encryption: "
              << std::fixed << std::setprecision(2) << enc_rate << " MB/s ("
              << enc_mb << " MB in " << enc_sec << " s)\n";

    // 2. Decryption Throughput
    Timer dec_timer;
    for (size_t i = 0; i < num_chunks; ++i) {
        auto [pt, is_final] = receiver.decrypt_chunk(encrypted_chunks[i]);
        (void)pt;
        (void)is_final;
    }
    double dec_sec = dec_timer.elapsed_sec();
    double dec_rate = enc_mb / dec_sec;

    std::cout << "  Streaming Decryption: "
              << std::fixed << std::setprecision(2) << dec_rate << " MB/s ("
              << enc_mb << " MB in " << dec_sec << " s)\n";

    std::cout << "============================================================\n";

    MITHRIL_TEST_ASSERT(enc_rate > 50.0); // Expect at least 50 MB/s
    MITHRIL_TEST_ASSERT(dec_rate > 50.0);

    return mithril::test::report_and_exit("Performance: Throughput Benchmark");
}
