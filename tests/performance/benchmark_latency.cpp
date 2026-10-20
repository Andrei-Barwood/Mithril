// ============================================================================
// Mithril Tests - Performance: Cryptographic Operation Latency Benchmark
// ============================================================================
// File: tests/performance/benchmark_latency.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <mithril/key_exchange.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/stream_encryption.hpp>
#include <mithril/crypto_hash.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;
using mithril::test::Timer;

int main() {
    std::cout << "[BENCHMARK] Cryptographic Operation Latencies\n";
    std::cout << "============================================================\n";

    const int iterations = 1000;

    // 1. ECDH Shared Secret Derivation
    {
        KeyExchange alice;
        KeyExchange bob;
        const auto &bob_pk = bob.get_public_key();

        Timer timer;
        for (int i = 0; i < iterations; ++i) {
            auto secret = alice.derive_shared_secret(bob_pk);
            (void)secret;
        }
        double total_us = timer.elapsed_us();
        double avg_us = total_us / iterations;

        std::cout << "  ECDH Shared Secret (X25519):  "
                  << std::fixed << std::setprecision(2) << avg_us << " us / op ("
                  << static_cast<int>(1000000.0 / avg_us) << " ops/sec)\n";
        MITHRIL_TEST_ASSERT(avg_us < 5000.0); // Should be well under 5ms
    }

    // 2. Ed25519 Sign
    {
        DigitalSignature signer;
        std::vector<uint8_t> msg(64, 0x42);

        Timer timer;
        for (int i = 0; i < iterations; ++i) {
            auto sig = signer.sign(msg);
            (void)sig;
        }
        double total_us = timer.elapsed_us();
        double avg_us = total_us / iterations;

        std::cout << "  Ed25519 Sign:                  "
                  << std::fixed << std::setprecision(2) << avg_us << " us / op ("
                  << static_cast<int>(1000000.0 / avg_us) << " ops/sec)\n";
        MITHRIL_TEST_ASSERT(avg_us < 5000.0);
    }

    // 3. Ed25519 Verify
    {
        DigitalSignature signer;
        std::vector<uint8_t> msg(64, 0x42);
        auto sig = signer.sign(msg);
        const auto &vk = signer.get_verify_key();

        Timer timer;
        for (int i = 0; i < iterations; ++i) {
            bool ok = signer.verify(msg, sig, vk);
            (void)ok;
        }
        double total_us = timer.elapsed_us();
        double avg_us = total_us / iterations;

        std::cout << "  Ed25519 Verify:                "
                  << std::fixed << std::setprecision(2) << avg_us << " us / op ("
                  << static_cast<int>(1000000.0 / avg_us) << " ops/sec)\n";
        MITHRIL_TEST_ASSERT(avg_us < 5000.0);
    }

    // 4. BLAKE2b-256 Hashing (1 KB)
    {
        std::vector<uint8_t> block(1024, 0x33);

        Timer timer;
        for (int i = 0; i < iterations; ++i) {
            auto h = CryptoHash::hash(block);
            (void)h;
        }
        double total_us = timer.elapsed_us();
        double avg_us = total_us / iterations;

        std::cout << "  BLAKE2b-256 (1 KB block):      "
                  << std::fixed << std::setprecision(2) << avg_us << " us / op ("
                  << static_cast<int>(1000000.0 / avg_us) << " ops/sec)\n";
        MITHRIL_TEST_ASSERT(avg_us < 1000.0);
    }

    // 5. AEAD Encrypt & Decrypt (1 KB)
    {
        AuthenticatedEncryption crypto;
        std::vector<uint8_t> block(1024, 0x77);

        Timer timer;
        for (int i = 0; i < iterations; ++i) {
            auto ct = crypto.encrypt(block);
            auto pt = crypto.decrypt(ct);
            (void)pt;
        }
        double total_us = timer.elapsed_us();
        double avg_us = total_us / iterations;

        std::cout << "  AEAD Encrypt + Decrypt (1 KB): "
                  << std::fixed << std::setprecision(2) << avg_us << " us / op ("
                  << static_cast<int>(1000000.0 / avg_us) << " ops/sec)\n";
        MITHRIL_TEST_ASSERT(avg_us < 1000.0);
    }

    std::cout << "============================================================\n";
    return mithril::test::report_and_exit("Performance: Latency Benchmark");
}
