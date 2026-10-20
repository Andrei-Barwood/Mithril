// ============================================================================
// Mithril Tests - Unit: Key Exchange (X25519)
// ============================================================================
// File: tests/unit/test_key_exchange.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <mithril/key_exchange.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Unit Tests: Key Exchange (X25519)\n";

    // 1. Keypair generation and non-triviality
    {
        std::cout << "  1. Keypair generation and key size...\n";
        KeyExchange alice;
        const auto &pk = alice.get_public_key();

        MITHRIL_TEST_ASSERT_EQ(pk.size(), MITHRIL_PUBLIC_KEY_BYTES);

        // Verify key is not all zeros
        bool all_zeros = true;
        for (size_t i = 0; i < pk.size(); ++i) {
            if (pk.data()[i] != 0) {
                all_zeros = false;
                break;
            }
        }
        MITHRIL_TEST_ASSERT(!all_zeros);
    }

    // 2. ECDH Shared Secret Derivation
    {
        std::cout << "  2. Two-party ECDH shared secret derivation...\n";
        KeyExchange alice;
        KeyExchange bob;

        auto secret_ab = alice.derive_shared_secret(bob.get_public_key());
        auto secret_ba = bob.derive_shared_secret(alice.get_public_key());

        MITHRIL_TEST_ASSERT_EQ(secret_ab, secret_ba);

        // Third party should derive a completely distinct secret
        KeyExchange charlie;
        auto secret_ac = alice.derive_shared_secret(charlie.get_public_key());
        MITHRIL_TEST_ASSERT_NE(secret_ab, secret_ac);
    }

    // 3. Direct asymmetric encrypt_to and decrypt_from
    {
        std::cout << "  3. Asymmetric encrypt_to and decrypt_from...\n";
        KeyExchange alice;
        KeyExchange bob;

        std::vector<uint8_t> message = {'H', 'e', 'l', 'l', 'o', ' ', 'B', 'o', 'b', '!'};

        // Alice encrypts for Bob
        auto ciphertext = alice.encrypt_to(message, bob.get_public_key());
        
        // Bob decrypts from Alice
        auto decrypted = bob.decrypt_from(ciphertext, alice.get_public_key());
        MITHRIL_TEST_ASSERT_EQ(decrypted, message);

        // Charlie tries to decrypt (should fail)
        KeyExchange charlie;
        MITHRIL_TEST_ASSERT_THROWS(charlie.decrypt_from(ciphertext, alice.get_public_key()), std::runtime_error);

        // Tampering with ciphertext should fail
        auto tampered = ciphertext;
        tampered[tampered.size() - 1] ^= 0x55;
        MITHRIL_TEST_ASSERT_THROWS(bob.decrypt_from(tampered, alice.get_public_key()), std::runtime_error);
    }

    return mithril::test::report_and_exit("Unit: Key Exchange");
}
