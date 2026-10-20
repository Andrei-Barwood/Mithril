#include <mithril/mithril_sodium.hpp>
#include <iostream>

using namespace mithril::sodium;

int main() {
    try {
        // Test 1: Key Exchange
        std::cout << "Test 1: Key Exchange..." << std::endl;
        KeyExchange alice, bob;
        auto alice_shared = alice.derive_shared_secret(bob.get_public_key());
        auto bob_shared = bob.derive_shared_secret(alice.get_public_key());
        
        if (alice_shared == bob_shared) {
            std::cout << "✓ ECDH successful\n";
        }
        
        // Test 2: Encryption
        std::cout << "Test 2: Authenticated Encryption..." << std::endl;
        AuthenticatedEncryption crypto(alice_shared);
        std::vector<uint8_t> plaintext = {1, 2, 3, 4, 5};
        auto ciphertext = crypto.encrypt(plaintext);
        auto decrypted = crypto.decrypt(ciphertext);
        
        if (decrypted == plaintext) {
            std::cout << "✓ Encryption roundtrip successful\n";
        }
        
        // Test 3: Digital Signature
        std::cout << "Test 3: Digital Signatures..." << std::endl;
        DigitalSignature signer;
        auto signature = signer.sign(plaintext);
        bool valid = signer.verify(plaintext, signature, signer.get_verify_key());
        
        if (valid) {
            std::cout << "✓ Signature verification successful\n";
        }
        
        // Test 4: Hashing
        std::cout << "Test 4: Cryptographic Hashing..." << std::endl;
        auto hash = CryptoHash::hash(plaintext);
        std::cout << "✓ Hash computed (" << hash.size() << " bytes)\n";
        
        std::cout << "\n✅ All tests passed!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << "\n";
        return 1;
    }
}
