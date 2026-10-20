// ============================================================================
// Mithril Tests - Unit: Digital Signatures (Ed25519)
// ============================================================================
// File: tests/unit/test_signatures.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <mithril/digital_signature.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>
#include "../test_helper.hpp"

using namespace mithril::sodium;

int main() {
    std::cout << "[TEST] Starting Unit Tests: Digital Signatures (Ed25519)\n";

    // 1. Keypair generation & verify valid signature
    {
        std::cout << "  1. Keypair generation and signature verification...\n";
        DigitalSignature signer;
        const auto &vk = signer.get_verify_key();

        MITHRIL_TEST_ASSERT_EQ(vk.size(), MITHRIL_SIGN_PUBLIC_KEY_BYTES);

        std::vector<uint8_t> message = {'C', 'o', 'm', 'm', 'a', 'n', 'd', ':', 'G', 'R', 'I', 'D', '_', 'O', 'N'};
        auto signature = signer.sign(message);

        MITHRIL_TEST_ASSERT_EQ(signature.size(), MITHRIL_SIGN_SIGNATURE_BYTES);

        bool valid = signer.verify(message, signature, vk);
        MITHRIL_TEST_ASSERT(valid);
    }

    // 2. Reject tampered message
    {
        std::cout << "  2. Tampered message rejection...\n";
        DigitalSignature signer;
        std::vector<uint8_t> message = {'A', 'u', 't', 'h', 'o', 'r', 'i', 'z', 'e', 'd'};
        auto signature = signer.sign(message);

        auto tampered_msg = message;
        tampered_msg[0] ^= 0x01;

        bool valid = signer.verify(tampered_msg, signature, signer.get_verify_key());
        MITHRIL_TEST_ASSERT(!valid);
    }

    // 3. Reject wrong verification key
    {
        std::cout << "  3. Wrong verification key rejection...\n";
        DigitalSignature signer1;
        DigitalSignature signer2;

        std::vector<uint8_t> message = {'D', 'a', 't', 'a'};
        auto signature = signer1.sign(message);

        // Verifying with signer2's key must fail
        bool valid = signer1.verify(message, signature, signer2.get_verify_key());
        MITHRIL_TEST_ASSERT(!valid);
    }

    // 4. Reject corrupted signature
    {
        std::cout << "  4. Corrupted signature rejection...\n";
        DigitalSignature signer;
        std::vector<uint8_t> message = {'T', 'e', 's', 't'};
        auto signature = signer.sign(message);

        signature[0] ^= 0x80; // Corrupt first byte of signature

        bool valid = signer.verify(message, signature, signer.get_verify_key());
        MITHRIL_TEST_ASSERT(!valid);
    }

    // 5. Sign and Encrypt integration
    {
        std::cout << "  5. Sign-and-encrypt authenticated channel...\n";
        DigitalSignature signer;
        
        // Recipient has ECDH keys
        KeyExchange recipient;
        // Sender has ECDH keys for transport
        KeyExchange sender_kex;

        // Since KeyExchange doesn't expose secret_key directly for security,
        // we test digital signature combined with AuthenticatedEncryption
        std::vector<uint8_t> plaintext = {'T', 'i', 'd', 'a', 'l', ' ', 'S', 'e', 'n', 's', 'o', 'r', ' ', 'D', 'a', 't', 'a'};
        auto sig = signer.sign(plaintext);

        auto shared = sender_kex.derive_shared_secret(recipient.get_public_key());
        AuthenticatedEncryption crypto(shared);

        std::vector<uint8_t> packet;
        packet.insert(packet.end(), sig.begin(), sig.end());
        packet.insert(packet.end(), plaintext.begin(), plaintext.end());

        auto encrypted_packet = crypto.encrypt(packet);

        // Recipient side
        auto recipient_shared = recipient.derive_shared_secret(sender_kex.get_public_key());
        AuthenticatedEncryption recipient_crypto(recipient_shared);

        auto decrypted_packet = recipient_crypto.decrypt(encrypted_packet);
        MITHRIL_TEST_ASSERT_EQ(decrypted_packet.size(), sig.size() + plaintext.size());

        Signature recv_sig{};
        std::copy_n(decrypted_packet.begin(), recv_sig.size(), recv_sig.begin());

        std::vector<uint8_t> recv_plaintext(decrypted_packet.begin() + recv_sig.size(), decrypted_packet.end());
        MITHRIL_TEST_ASSERT_EQ(recv_plaintext, plaintext);

        bool sig_ok = signer.verify(recv_plaintext, recv_sig, signer.get_verify_key());
        MITHRIL_TEST_ASSERT(sig_ok);
    }

    return mithril::test::report_and_exit("Unit: Digital Signatures");
}
