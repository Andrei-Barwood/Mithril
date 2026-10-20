// ============================================================================
// Mithril Tests - Security: Anti-Replay Attack Protection
// ============================================================================
// File: tests/security/test_replay_attack.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Security Test: Anti-Replay Attack Protection\n";

    // 1. Cross-Session Replay Immunity via Ephemeral ECDH
    {
        std::cout << "  1. Cross-session replay attack immunity...\n";
        // Session 1: Alice & Bob establish ephemeral keys
        KeyExchange alice1, bob1;
        auto shared1_a = alice1.derive_shared_secret(bob1.get_public_key());
        auto shared1_b = bob1.derive_shared_secret(alice1.get_public_key());
        AuthenticatedEncryption crypto1_a(shared1_a);
        AuthenticatedEncryption crypto1_b(shared1_b);

        std::vector<uint8_t> critical_cmd = {'T', 'R', 'A', 'N', 'S', 'F', 'E', 'R', '_', '1', '0', '0', '0'};
        auto intercepted_packet = crypto1_a.encrypt(critical_cmd);

        // Bob1 decrypts successfully in Session 1
        auto plain1 = crypto1_b.decrypt(intercepted_packet);
        MITHRIL_TEST_ASSERT_EQ(plain1, critical_cmd);

        // Session 2: Fresh ephemeral key exchange between Alice & Bob
        KeyExchange alice2, bob2;
        auto shared2_b = bob2.derive_shared_secret(alice2.get_public_key());
        AuthenticatedEncryption crypto2_b(shared2_b);

        // Attacker replays intercepted_packet into Session 2!
        // Decryption MUST fail and throw an exception.
        MITHRIL_TEST_ASSERT_THROWS(crypto2_b.decrypt(intercepted_packet), std::runtime_error);
        std::cout << "     ✓ Replayed packet from past session rejected by Session 2.\n";
    }

    // 2. Stream chunk replay / duplication attack
    {
        std::cout << "  2. Stream chunk duplicate replay attack...\n";
        StreamEncryption sender;
        StreamEncryption receiver(sender.get_key());
        receiver.init_pull_state(sender.get_header());

        std::vector<uint8_t> chunk0 = {'C', 'h', 'u', 'n', 'k', '_', '0'};
        std::vector<uint8_t> chunk1 = {'C', 'h', 'u', 'n', 'k', '_', '1'};

        auto enc0 = sender.encrypt_chunk(chunk0, false);
        auto enc1 = sender.encrypt_chunk(chunk1, false);

        // Receiver consumes chunk 0
        auto [dec0, f0] = receiver.decrypt_chunk(enc0);
        MITHRIL_TEST_ASSERT_EQ(dec0, chunk0);

        // Attacker injects duplicate enc0 again instead of enc1!
        // Receiver's internal pull counter is now at 1, but enc0 used counter 0.
        // Decryption with mismatched counter must fail.
        MITHRIL_TEST_ASSERT_THROWS(receiver.decrypt_chunk(enc0), std::runtime_error);
        std::cout << "     ✓ Duplicate replayed stream chunk rejected.\n";
    }

    return mithril::test::report_and_exit("Security: Anti-Replay Protection");
}
