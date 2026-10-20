// ============================================================================
// Mithril Tests - Unit: Authenticated & Stream Encryption
// ============================================================================
// File: tests/unit/test_encryption.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Unit Tests: Authenticated & Stream Encryption\n";

    // 1. AuthenticatedEncryption: Key initialization & roundtrip
    {
        std::cout << "  1. AuthenticatedEncryption roundtrip on multiple sizes...\n";
        AuthenticatedEncryption crypto;
        
        const std::vector<size_t> test_sizes = {0, 1, 16, 64, 256, 1024, 65536};
        for (size_t sz : test_sizes) {
            auto plaintext = mithril::test::make_pattern(sz, static_cast<uint8_t>(sz));
            auto ciphertext = crypto.encrypt(plaintext);

            MITHRIL_TEST_ASSERT_EQ(ciphertext.size(), plaintext.size() + MITHRIL_AEAD_NONCE_BYTES + MITHRIL_AEAD_TAG_BYTES);
            
            auto decrypted = crypto.decrypt(ciphertext);
            MITHRIL_TEST_ASSERT_EQ(decrypted, plaintext);
        }
    }

    // 2. Nonce Randomization check
    {
        std::cout << "  2. Nonce randomization check...\n";
        AuthenticatedEncryption crypto;
        std::vector<uint8_t> msg = {'s', 'e', 'c', 'r', 'e', 't', ' ', 'd', 'a', 't', 'a'};

        auto c1 = crypto.encrypt(msg);
        auto c2 = crypto.encrypt(msg);

        // Ciphertexts must be distinct because nonces are randomly generated
        MITHRIL_TEST_ASSERT_NE(c1, c2);
        MITHRIL_TEST_ASSERT_EQ(crypto.decrypt(c1), msg);
        MITHRIL_TEST_ASSERT_EQ(crypto.decrypt(c2), msg);
    }

    // 3. Tampering and Truncation Detection
    {
        std::cout << "  3. Tampering and truncation detection...\n";
        AuthenticatedEncryption crypto;
        std::vector<uint8_t> msg = {'p', 'a', 'y', 'l', 'o', 'a', 'd'};
        auto ciphertext = crypto.encrypt(msg);

        // Tamper with one byte in the ciphertext body
        auto tampered = ciphertext;
        tampered[tampered.size() - 1] ^= 0x01;
        MITHRIL_TEST_ASSERT_THROWS(crypto.decrypt(tampered), std::runtime_error);

        // Tamper with nonce
        auto tampered_nonce = ciphertext;
        tampered_nonce[0] ^= 0xFF;
        MITHRIL_TEST_ASSERT_THROWS(crypto.decrypt(tampered_nonce), std::runtime_error);

        // Truncate below minimum valid length (nonce + tag)
        std::vector<uint8_t> short_ct(MITHRIL_AEAD_NONCE_BYTES + MITHRIL_AEAD_TAG_BYTES - 1, 0xAA);
        MITHRIL_TEST_ASSERT_THROWS(crypto.decrypt(short_ct), std::invalid_argument);
    }

    // 4. StreamEncryption: Sequential chunks and final flag
    {
        std::cout << "  4. StreamEncryption sequential chunk streaming...\n";
        StreamEncryption sender;
        StreamEncryption receiver(sender.get_key());

        // Initialize receiver with sender's header
        receiver.init_pull_state(sender.get_header());

        std::vector<uint8_t> chunk1 = {'c', 'h', 'u', 'n', 'k', '1'};
        std::vector<uint8_t> chunk2 = {'c', 'h', 'u', 'n', 'k', '2', '-', 'l', 'a', 'r', 'g', 'e', 'r'};
        std::vector<uint8_t> chunk3 = {'e', 'n', 'd'};

        auto enc1 = sender.encrypt_chunk(chunk1, false);
        auto enc2 = sender.encrypt_chunk(chunk2, false);
        auto enc3 = sender.encrypt_chunk(chunk3, true); // final chunk

        auto [dec1, fin1] = receiver.decrypt_chunk(enc1);
        MITHRIL_TEST_ASSERT_EQ(dec1, chunk1);
        MITHRIL_TEST_ASSERT_EQ(fin1, false);

        auto [dec2, fin2] = receiver.decrypt_chunk(enc2);
        MITHRIL_TEST_ASSERT_EQ(dec2, chunk2);
        MITHRIL_TEST_ASSERT_EQ(fin2, false);

        auto [dec3, fin3] = receiver.decrypt_chunk(enc3);
        MITHRIL_TEST_ASSERT_EQ(dec3, chunk3);
        MITHRIL_TEST_ASSERT_EQ(fin3, true);
    }

    // 5. StreamEncryption: Uninitialized pull state check
    {
        std::cout << "  5. StreamEncryption uninitialized pull state...\n";
        StreamEncryption uninit_receiver;
        std::vector<uint8_t> dummy(32, 0x11);
        MITHRIL_TEST_ASSERT_THROWS(uninit_receiver.decrypt_chunk(dummy), std::runtime_error);
    }

    return mithril::test::report_and_exit("Unit: Authenticated & Stream Encryption");
}
