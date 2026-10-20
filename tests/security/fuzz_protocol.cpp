// ============================================================================
// Mithril Tests - Security: Protocol Robustness & Mutational Fuzzing
// ============================================================================
// File: tests/security/fuzz_protocol.cpp
// ============================================================================

#include <iostream>
#include <random>
#include <vector>
#include <mithril/key_exchange.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/stream_encryption.hpp>
#include <mithril/crypto_hash.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Security Test: Mutational Fuzzing & Boundary Testing\n";

    std::mt19937 rng(42); // Deterministic seed for reproducible fuzz testing
    std::uniform_int_distribution<size_t> len_dist(0, 512);
    std::uniform_int_distribution<uint32_t> byte_dist(0, 255);

    AuthenticatedEncryption aead;
    DigitalSignature sig;
    KeyExchange kex;

    const size_t num_iterations = 500;
    size_t exceptions_caught = 0;

    std::cout << "  1. Fuzzing AuthenticatedEncryption::decrypt with random bytes (" << num_iterations << " iterations)...\n";
    for (size_t i = 0; i < num_iterations; ++i) {
        size_t len = len_dist(rng);
        std::vector<uint8_t> random_payload(len);
        for (size_t j = 0; j < len; ++j) {
            random_payload[j] = static_cast<uint8_t>(byte_dist(rng));
        }

        try {
            // Decrypting arbitrary fuzzed data must never crash/segfault
            auto out = aead.decrypt(random_payload);
            (void)out;
        } catch (const std::exception &) {
            ++exceptions_caught;
        }
    }
    std::cout << "     ✓ Handled " << exceptions_caught << " exceptions safely without crash.\n";

    std::cout << "  2. Fuzzing DigitalSignature::verify with corrupted inputs (" << num_iterations << " iterations)...\n";
    std::vector<uint8_t> real_msg = {'T', 'e', 's', 't', 'M', 's', 'g'};
    auto real_sig = sig.sign(real_msg);

    for (size_t i = 0; i < num_iterations; ++i) {
        auto mutated_sig = real_sig;
        // Mutate 1 to 5 random bytes
        size_t mutations = 1 + (byte_dist(rng) % 5);
        for (size_t m = 0; m < mutations; ++m) {
            size_t idx = byte_dist(rng) % mutated_sig.size();
            mutated_sig[idx] ^= static_cast<uint8_t>(1 + (byte_dist(rng) % 254));
        }

        bool ok = sig.verify(real_msg, mutated_sig, sig.get_verify_key());
        MITHRIL_TEST_ASSERT(!ok);
    }
    std::cout << "     ✓ All " << num_iterations << " mutated signatures correctly rejected.\n";

    std::cout << "  3. Fuzzing CryptoHash::hash with variable boundary buffers...\n";
    const std::vector<size_t> boundary_sizes = {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 1024, 65535};
    for (size_t sz : boundary_sizes) {
        std::vector<uint8_t> buf(sz, 0xEE);
        auto h = CryptoHash::hash(buf);
        MITHRIL_TEST_ASSERT_EQ(h.size(), MITHRIL_HASH_BYTES);
    }
    std::cout << "     ✓ Boundary buffers hashed cleanly.\n";

    return mithril::test::report_and_exit("Security: Protocol Fuzzing");
}
