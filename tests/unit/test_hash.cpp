// ============================================================================
// Mithril Tests - Unit: Cryptographic Hashing
// ============================================================================
// File: tests/unit/test_hash.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <bitset>
#include <mithril/crypto_hash.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Unit Tests: Cryptographic Hashing (BLAKE2b)\n";

    // 1. Basic Hash & Determinism
    {
        std::cout << "  1. Basic digest computation and determinism...\n";
        std::vector<uint8_t> data = {'M', 'i', 't', 'h', 'r', 'i', 'l', ' ', 'H', 'a', 's', 'h'};

        auto h1 = CryptoHash::hash(data);
        auto h2 = CryptoHash::hash(data);

        MITHRIL_TEST_ASSERT_EQ(h1.size(), MITHRIL_HASH_BYTES);
        MITHRIL_TEST_ASSERT_EQ(h1, h2);
    }

    // 2. Avalanche Effect: 1-bit input change yields dramatically different output
    {
        std::cout << "  2. Avalanche effect test...\n";
        std::vector<uint8_t> data1 = {'P', 'r', 'o', 't', 'o', 'c', 'o', 'l', ' ', 'V', '2'};
        std::vector<uint8_t> data2 = data1;
        data2[0] ^= 0x01; // flip 1 bit

        auto h1 = CryptoHash::hash(data1);
        auto h2 = CryptoHash::hash(data2);

        int bit_diff_count = 0;
        for (size_t i = 0; i < h1.size(); ++i) {
            bit_diff_count += std::bitset<8>(h1[i] ^ h2[i]).count();
        }

        // Out of 256 bits, ideally ~128 bits flip. Expect at least 64 bits to flip (>25%)
        std::cout << "     Bit difference: " << bit_diff_count << " / 256 bits\n";
        MITHRIL_TEST_ASSERT(bit_diff_count >= 64);
    }

    // 3. Keyed Hash
    {
        std::cout << "  3. Keyed hash computation...\n";
        std::vector<uint8_t> data = {'S', 'e', 'n', 's', 'o', 'r', ' ', 'D', 'a', 't', 'a'};
        std::vector<uint8_t> key1 = {1, 2, 3, 4, 5, 6, 7, 8};
        std::vector<uint8_t> key2 = {8, 7, 6, 5, 4, 3, 2, 1};

        auto kh1 = CryptoHash::keyed_hash(data, key1);
        auto kh2 = CryptoHash::keyed_hash(data, key2);
        auto unkeyed = CryptoHash::hash(data);

        MITHRIL_TEST_ASSERT_NE(kh1, kh2);
        MITHRIL_TEST_ASSERT_NE(kh1, unkeyed);
    }

    // 4. MultiPartHash Streaming equivalence
    {
        std::cout << "  4. MultiPartHash streaming equivalence...\n";
        std::vector<uint8_t> part1 = {'H', 'e', 'a', 'd', 'e', 'r', ':'};
        std::vector<uint8_t> part2 = {'P', 'a', 'y', 'l', 'o', 'a', 'd'};
        std::vector<uint8_t> part3 = {';', 'T', 'a', 'i', 'l'};

        std::vector<uint8_t> combined;
        combined.insert(combined.end(), part1.begin(), part1.end());
        combined.insert(combined.end(), part2.begin(), part2.end());
        combined.insert(combined.end(), part3.begin(), part3.end());

        CryptoHash::MultiPartHash hasher;
        hasher.update(part1);
        hasher.update(part2);
        hasher.update(part3);
        auto multi_hash = hasher.finalize();

        auto contiguous_hash = CryptoHash::hash(combined);

        MITHRIL_TEST_ASSERT_EQ(multi_hash, contiguous_hash);
    }

    // 5. Utility: Hex formatting
    {
        std::cout << "  5. Hex conversion roundtrip...\n";
        std::vector<uint8_t> sample = {0x00, 0x12, 0xAB, 0xCD, 0xEF, 0xFF};
        std::string hex_str = util::to_hex(sample);
        MITHRIL_TEST_ASSERT_EQ(hex_str, "0012abcdefff");

        std::vector<uint8_t> parsed = util::from_hex(hex_str);
        MITHRIL_TEST_ASSERT_EQ(parsed, sample);
    }

    return mithril::test::report_and_exit("Unit: Cryptographic Hashing");
}
