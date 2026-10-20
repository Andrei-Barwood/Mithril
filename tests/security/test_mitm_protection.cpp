// ============================================================================
// Mithril Tests - Security: Man-in-the-Middle (MitM) Protection
// ============================================================================
// File: tests/security/test_mitm_protection.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <mithril/key_exchange.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Security Test: Man-in-the-Middle (MitM) Protection\n";

    // 1. MitM Key Substitution Detection
    {
        std::cout << "  1. Key substitution attack detection...\n";
        KeyExchange alice;
        KeyExchange bob;
        KeyExchange eve; // Attacker

        // Alice attempts to send public key to Bob, but Eve intercepts and replaces with Eve's PK
        const auto &eve_substituted_pk = eve.get_public_key();

        // Bob unknowingly derives shared secret with Eve
        auto bob_eve_secret = bob.derive_shared_secret(eve_substituted_pk);
        // Alice derives shared secret with Bob
        auto alice_bob_secret = alice.derive_shared_secret(bob.get_public_key());

        // The derived secrets do NOT match
        MITHRIL_TEST_ASSERT_NE(alice_bob_secret, bob_eve_secret);

        AuthenticatedEncryption bob_crypto(bob_eve_secret);
        AuthenticatedEncryption alice_crypto(alice_bob_secret);

        // A message encrypted by Bob cannot be decrypted by Alice
        std::vector<uint8_t> bob_msg = {'T', 'u', 'r', 'b', 'i', 'n', 'e', '_', 'O', 'K'};
        auto ct = bob_crypto.encrypt(bob_msg);

        MITHRIL_TEST_ASSERT_THROWS(alice_crypto.decrypt(ct), std::runtime_error);
        std::cout << "     ✓ Alice rejected payload encrypted with substituted key.\n";
    }

    // 2. Cryptographic Defense: Ephemeral Key Authenticated by Static Ed25519 Identity
    {
        std::cout << "  2. Static signature authentication defeating MitM key replacement...\n";
        DigitalSignature alice_identity;
        KeyExchange alice_ephemeral;

        // Alice signs her ephemeral public key with her permanent identity key
        const auto &alice_kex_pk = alice_ephemeral.get_public_key();
        std::span<const uint8_t> pk_span(alice_kex_pk.data(), alice_kex_pk.size());
        auto sig = alice_identity.sign(pk_span);

        // Eve tries to substitute with Eve's ephemeral public key
        KeyExchange eve_ephemeral;
        const auto &eve_kex_pk = eve_ephemeral.get_public_key();
        std::span<const uint8_t> eve_span(eve_kex_pk.data(), eve_kex_pk.size());

        // Bob verifies the received ephemeral key against Alice's known trusted identity key
        bool valid_alice = alice_identity.verify(pk_span, sig, alice_identity.get_verify_key());
        MITHRIL_TEST_ASSERT(valid_alice);

        // Eve's substituted key will fail verification with Alice's identity signature
        bool valid_eve = alice_identity.verify(eve_span, sig, alice_identity.get_verify_key());
        MITHRIL_TEST_ASSERT(!valid_eve);
        std::cout << "     ✓ Bob detected forged ephemeral key from Eve via Ed25519 signature.\n";
    }

    return mithril::test::report_and_exit("Security: MitM Protection");
}
