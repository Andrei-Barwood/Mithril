// ============================================================================
// Mithril Network Layer - libsodium Wrapper
// ============================================================================
// File: network/include/mithril/mithril_sodium.hpp
// Author: Andrei Barwood
// Date: 2025-11-05
// Version: 1.0.0
//
// Description:
//   Complete C++20 wrapper for libsodium cryptographic primitives
//   Designed for IoT devices and secure network communication
//
// Features:
//   - RAII-based secure key management
//   - Memory-locked keys (prevents swapping)
//   - Curve25519 key exchange (ECDH)
//   - ChaCha20-Poly1305 authenticated encryption
//   - Ed25519 digital signatures
//   - BLAKE2b hashing
//   - Streaming encryption for large data
//
// Dependencies:
//   - C++20 compiler
//   - libsodium >= 1.0.18
//   - <span>, <concepts> from C++20
//
// Usage:
//   #include <mithril/mithril_sodium.hpp>
//   using namespace mithril::sodium;
//
// ============================================================================

#pragma once

#ifndef MITHRIL_SODIUM_HPP
#define MITHRIL_SODIUM_HPP

// Standard library
#include <array>
#include <vector>
#include <span>
#include <memory>
#include <stdexcept>
#include <concepts>
#include <type_traits>
#include <string_view>
#include <cstring>
#include <algorithm>

// libsodium
#include <sodium.h>

namespace mithril::sodium {

// ============================================================================
// Forward Declarations
// ============================================================================

template<size_t N>
class SecureKey;

class SodiumInit;
class KeyExchange;
class AuthenticatedEncryption;
class StreamEncryption;
class DigitalSignature;
class CryptoHash;

// ============================================================================
// Type Aliases for Convenience
// ============================================================================

// Key types with proper sizes
using PublicKey = SecureKey<crypto_box_PUBLICKEYBYTES>;       // 32 bytes
using SecretKey = SecureKey<crypto_box_SECRETKEYBYTES>;       // 32 bytes
using SessionKey = SecureKey<crypto_secretbox_KEYBYTES>;      // 32 bytes
using SigningKey = SecureKey<crypto_sign_SECRETKEYBYTES>;     // 64 bytes
using VerifyKey = SecureKey<crypto_sign_PUBLICKEYBYTES>;      // 32 bytes
using SymmetricKey = SecureKey<crypto_secretstream_xchacha20poly1305_KEYBYTES>; // 32 bytes

// Nonce types
using Nonce = std::array<uint8_t, crypto_secretbox_NONCEBYTES>;
using StreamHeader = std::array<uint8_t, crypto_secretstream_xchacha20poly1305_HEADERBYTES>;

// Hash types
using Hash256 = std::array<uint8_t, crypto_generichash_BYTES>;
using Signature = std::array<uint8_t, crypto_sign_BYTES>;

// ============================================================================
// Concepts for Type Safety
// ============================================================================

template<typename T>
concept SecureKeyType = requires(T key) {
    { key.data() } -> std::convertible_to<uint8_t*>;
    { key.size() } -> std::convertible_to<size_t>;
    { key.span() } -> std::convertible_to<std::span<uint8_t>>;
};

template<typename T>
concept ByteSpan = std::convertible_to<T, std::span<const uint8_t>>;

// ============================================================================
// Global libsodium Initialization (RAII)
// ============================================================================

class SodiumInit {
public:
    SodiumInit() {
        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize libsodium");
        }
    }
    
    // Singleton pattern
    static SodiumInit& instance() {
        static SodiumInit init;
        return init;
    }
    
    // Delete copy/move
    SodiumInit(const SodiumInit&) = delete;
    SodiumInit& operator=(const SodiumInit&) = delete;
    SodiumInit(SodiumInit&&) = delete;
    SodiumInit& operator=(SodiumInit&&) = delete;
};

// ============================================================================
// Secure Key Template (RAII + Memory Locking)
// ============================================================================

template<size_t N>
class SecureKey {
public:
    // Constructor: Lock memory to prevent swapping
    SecureKey() {
        if (sodium_mlock(data_.data(), data_.size()) != 0) {
            throw std::runtime_error("Failed to lock memory for secure key");
        }
    }
    
    // Destructor: Unlock and zero memory
    ~SecureKey() {
        sodium_munlock(data_.data(), data_.size());
    }
    
    // No copy (security: prevent key duplication)
    SecureKey(const SecureKey&) = delete;
    SecureKey& operator=(const SecureKey&) = delete;
    
    // Move allowed (transfer ownership)
    SecureKey(SecureKey&& other) noexcept {
        std::copy(other.data_.begin(), other.data_.end(), data_.begin());
        sodium_memzero(other.data_.data(), other.data_.size());
    }
    
    SecureKey& operator=(SecureKey&& other) noexcept {
        if (this != &other) {
            std::copy(other.data_.begin(), other.data_.end(), data_.begin());
            sodium_memzero(other.data_.data(), other.data_.size());
        }
        return *this;
    }
    
    // Accessors
    uint8_t* data() noexcept { return data_.data(); }
    const uint8_t* data() const noexcept { return data_.data(); }
    constexpr size_t size() const noexcept { return N; }
    
    std::span<uint8_t> span() noexcept { 
        return {data_.data(), N}; 
    }
    
    std::span<const uint8_t> span() const noexcept { 
        return {data_.data(), N}; 
    }
    
    // Generate random key
    void randomize() {
        randombytes_buf(data_.data(), data_.size());
    }
    
    // Secure comparison (constant-time)
    bool operator==(const SecureKey& other) const noexcept {
        return sodium_memcmp(data_.data(), other.data_.data(), N) == 0;
    }
    
    bool operator!=(const SecureKey& other) const noexcept {
        return !(*this == other);
    }

private:
    std::array<uint8_t, N> data_;
};

// ============================================================================
// Key Exchange (Curve25519 ECDH)
// ============================================================================

class KeyExchange {
public:
    KeyExchange() {
        SodiumInit::instance(); // Ensure libsodium is initialized
        
        if (crypto_box_keypair(public_key_.data(), secret_key_.data()) != 0) {
            throw std::runtime_error("Failed to generate key pair");
        }
    }
    
    // Get public key (safe to share)
    const PublicKey& get_public_key() const noexcept {
        return public_key_;
    }
    
    // Derive shared secret using ECDH (Curve25519)
    SessionKey derive_shared_secret(const PublicKey& peer_public_key) const {
        SessionKey shared;
        
        if (crypto_box_beforenm(
                shared.data(),
                peer_public_key.data(),
                secret_key_.data()) != 0) {
            throw std::runtime_error("Key exchange failed");
        }
        
        return shared;
    }
    
    // Public key encryption (for initial handshake)
    std::vector<uint8_t> encrypt_to(
        std::span<const uint8_t> plaintext,
        const PublicKey& recipient_public_key) const {
        
        Nonce nonce;
        randombytes_buf(nonce.data(), nonce.size());
        
        std::vector<uint8_t> ciphertext(
            nonce.size() + plaintext.size() + crypto_box_MACBYTES
        );
        
        // Include nonce at the beginning
        std::copy(nonce.begin(), nonce.end(), ciphertext.begin());
        
        if (crypto_box_easy(
                ciphertext.data() + nonce.size(),
                plaintext.data(), plaintext.size(),
                nonce.data(),
                recipient_public_key.data(),
                secret_key_.data()) != 0) {
            throw std::runtime_error("Public key encryption failed");
        }
        
        return ciphertext;
    }
    
    // Public key decryption
    std::vector<uint8_t> decrypt_from(
        std::span<const uint8_t> ciphertext,
        const PublicKey& sender_public_key) const {
        
        if (ciphertext.size() < crypto_box_NONCEBYTES + crypto_box_MACBYTES) {
            throw std::invalid_argument("Ciphertext too short");
        }
        
        // Extract nonce
        Nonce nonce;
        std::copy_n(ciphertext.begin(), nonce.size(), nonce.begin());
        
        auto cipher_data = ciphertext.subspan(nonce.size());
        std::vector<uint8_t> plaintext(cipher_data.size() - crypto_box_MACBYTES);
        
        if (crypto_box_open_easy(
                plaintext.data(),
                cipher_data.data(), cipher_data.size(),
                nonce.data(),
                sender_public_key.data(),
                secret_key_.data()) != 0) {
            throw std::runtime_error("Public key decryption failed or message forged");
        }
        
        return plaintext;
    }

private:
    PublicKey public_key_;
    SecretKey secret_key_;
};

// ============================================================================
// Authenticated Encryption (ChaCha20-Poly1305)
// ============================================================================

class AuthenticatedEncryption {
public:
    // Constructor with provided key
    explicit AuthenticatedEncryption(const SessionKey& key) 
        : key_(key) {
        SodiumInit::instance();
    }
    
    // Constructor with random key generation
    AuthenticatedEncryption() {
        SodiumInit::instance();
        crypto_secretbox_keygen(key_.data());
    }
    
    // Encrypt with authentication (AEAD)
    std::vector<uint8_t> encrypt(std::span<const uint8_t> plaintext) const {
        Nonce nonce;
        randombytes_buf(nonce.data(), nonce.size());
        
        // Format: [nonce][ciphertext+mac]
        std::vector<uint8_t> result(
            nonce.size() + plaintext.size() + crypto_secretbox_MACBYTES
        );
        
        std::copy(nonce.begin(), nonce.end(), result.begin());
        
        if (crypto_secretbox_easy(
                result.data() + nonce.size(),
                plaintext.data(), plaintext.size(),
                nonce.data(), key_.data()) != 0) {
            throw std::runtime_error("Encryption failed");
        }
        
        return result;
    }
    
    // Decrypt with authentication verification
    std::vector<uint8_t> decrypt(std::span<const uint8_t> ciphertext) const {
        if (ciphertext.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
            throw std::invalid_argument("Ciphertext too short");
        }
        
        // Extract nonce
        Nonce nonce;
        std::copy_n(ciphertext.begin(), nonce.size(), nonce.begin());
        
        auto cipher_data = ciphertext.subspan(nonce.size());
        std::vector<uint8_t> plaintext(cipher_data.size() - crypto_secretbox_MACBYTES);
        
        if (crypto_secretbox_open_easy(
                plaintext.data(),
                cipher_data.data(), cipher_data.size(),
                nonce.data(), key_.data()) != 0) {
            throw std::runtime_error("Decryption failed or message authentication failed");
        }
        
        return plaintext;
    }
    
    // Get the session key
    const SessionKey& get_key() const noexcept {
        return key_;
    }

private:
    SessionKey key_;
};

// ============================================================================
// Stream Encryption (for large files or continuous data)
// ============================================================================

class StreamEncryption {
public:
    StreamEncryption() {
        SodiumInit::instance();
        crypto_secretstream_xchacha20poly1305_keygen(key_.data());
        init_push_state();
    }
    
    explicit StreamEncryption(const SymmetricKey& key) : key_(key) {
        SodiumInit::instance();
        init_push_state();
    }
    
    // Get header (send this first)
    const StreamHeader& get_header() const noexcept {
        return header_;
    }
    
    // Encrypt chunk of stream
    std::vector<uint8_t> encrypt_chunk(
        std::span<const uint8_t> chunk,
        bool is_final = false) {
        
        std::vector<uint8_t> ciphertext(
            chunk.size() + crypto_secretstream_xchacha20poly1305_ABYTES
        );
        
        unsigned long long ciphertext_len;
        uint8_t tag = is_final ? 
            crypto_secretstream_xchacha20poly1305_TAG_FINAL :
            crypto_secretstream_xchacha20poly1305_TAG_MESSAGE;
        
        if (crypto_secretstream_xchacha20poly1305_push(
                &state_push_, ciphertext.data(), &ciphertext_len,
                chunk.data(), chunk.size(),
                nullptr, 0, tag) != 0) {
            throw std::runtime_error("Stream encryption failed");
        }
        
        ciphertext.resize(ciphertext_len);
        return ciphertext;
    }
    
    // Initialize decryption stream
    void init_pull_state(const StreamHeader& header) {
        if (crypto_secretstream_xchacha20poly1305_init_pull(
                &state_pull_, header.data(), key_.data()) != 0) {
            throw std::runtime_error("Failed to initialize pull state");
        }
    }
    
    // Decrypt chunk of stream
    std::pair<std::vector<uint8_t>, bool> decrypt_chunk(
        std::span<const uint8_t> ciphertext) {
        
        std::vector<uint8_t> plaintext(ciphertext.size());
        unsigned long long plaintext_len;
        unsigned char tag;
        
        if (crypto_secretstream_xchacha20poly1305_pull(
                &state_pull_, plaintext.data(), &plaintext_len, &tag,
                ciphertext.data(), ciphertext.size(),
                nullptr, 0) != 0) {
            throw std::runtime_error("Stream decryption failed or corrupted");
        }
        
        plaintext.resize(plaintext_len);
        bool is_final = (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL);
        
        return {std::move(plaintext), is_final};
    }

private:
    void init_push_state() {
        if (crypto_secretstream_xchacha20poly1305_init_push(
                &state_push_, header_.data(), key_.data()) != 0) {
            throw std::runtime_error("Failed to initialize push state");
        }
    }

    SymmetricKey key_;
    crypto_secretstream_xchacha20poly1305_state state_push_;
    crypto_secretstream_xchacha20poly1305_state state_pull_;
    StreamHeader header_;
};

// ============================================================================
// Digital Signatures (Ed25519)
// ============================================================================

class DigitalSignature {
public:
    DigitalSignature() {
        SodiumInit::instance();
        
        if (crypto_sign_keypair(verify_key_.data(), signing_key_.data()) != 0) {
            throw std::runtime_error("Failed to generate signing key pair");
        }
    }
    
    // Get verification key (public)
    const VerifyKey& get_verify_key() const noexcept {
        return verify_key_;
    }
    
    // Sign a message
    Signature sign(std::span<const uint8_t> message) const {
        Signature signature;
        
        crypto_sign_detached(
            signature.data(), nullptr,
            message.data(), message.size(),
            signing_key_.data()
        );
        
        return signature;
    }
    
    // Verify a signature
    bool verify(
        std::span<const uint8_t> message,
        const Signature& signature,
        const VerifyKey& public_key) const noexcept {
        
        return crypto_sign_verify_detached(
            signature.data(),
            message.data(), message.size(),
            public_key.data()
        ) == 0;
    }
    
    // Sign and encrypt (combined operation)
    std::vector<uint8_t> sign_and_encrypt(
        std::span<const uint8_t> plaintext,
        const PublicKey& recipient_public_key,
        const SecretKey& sender_secret_key) const {
        
        // First sign
        auto signature = sign(plaintext);
        
        // Combine signature + plaintext
        std::vector<uint8_t> signed_data;
        signed_data.reserve(signature.size() + plaintext.size());
        signed_data.insert(signed_data.end(), signature.begin(), signature.end());
        signed_data.insert(signed_data.end(), plaintext.begin(), plaintext.end());
        
        // Then encrypt
        Nonce nonce;
        randombytes_buf(nonce.data(), nonce.size());
        
        std::vector<uint8_t> ciphertext(
            nonce.size() + signed_data.size() + crypto_box_MACBYTES
        );
        
        std::copy(nonce.begin(), nonce.end(), ciphertext.begin());
        
        if (crypto_box_easy(
                ciphertext.data() + nonce.size(),
                signed_data.data(), signed_data.size(),
                nonce.data(),
                recipient_public_key.data(),
                sender_secret_key.data()) != 0) {
            throw std::runtime_error("Sign-and-encrypt failed");
        }
        
        return ciphertext;
    }

private:
    VerifyKey verify_key_;
    SigningKey signing_key_;
};

// ============================================================================
// Cryptographic Hashing (BLAKE2b)
// ============================================================================

class CryptoHash {
public:
    // Simple hash
    static Hash256 hash(std::span<const uint8_t> data) {
        SodiumInit::instance();
        
        Hash256 result;
        crypto_generichash(
            result.data(), result.size(),
            data.data(), data.size(),
            nullptr, 0
        );
        return result;
    }
    
    // Keyed hash (MAC)
    static Hash256 keyed_hash(
        std::span<const uint8_t> data,
        std::span<const uint8_t> key) {
        
        SodiumInit::instance();
        
        Hash256 result;
        crypto_generichash(
            result.data(), result.size(),
            data.data(), data.size(),
            key.data(), key.size()
        );
        return result;
    }
    
    // Multi-part hashing (for streaming)
    class MultiPartHash {
    public:
        MultiPartHash() {
            SodiumInit::instance();
            crypto_generichash_init(&state_, nullptr, 0, crypto_generichash_BYTES);
        }
        
        void update(std::span<const uint8_t> data) {
            crypto_generichash_update(&state_, data.data(), data.size());
        }
        
        Hash256 finalize() {
            Hash256 result;
            crypto_generichash_final(&state_, result.data(), result.size());
            return result;
        }
    
    private:
        crypto_generichash_state state_;
    };
};

// ============================================================================
// Utility Functions
// ============================================================================

namespace util {

// Constant-time comparison
inline bool secure_compare(
    std::span<const uint8_t> a,
    std::span<const uint8_t> b) noexcept {
    
    if (a.size() != b.size()) return false;
    return sodium_memcmp(a.data(), b.data(), a.size()) == 0;
}

// Hex encoding
inline std::string to_hex(std::span<const uint8_t> data) {
    std::string hex(data.size() * 2 + 1, '\0');
    sodium_bin2hex(hex.data(), hex.size(), data.data(), data.size());
    hex.pop_back(); // Remove null terminator
    return hex;
}

// Hex decoding
inline std::vector<uint8_t> from_hex(std::string_view hex) {
    std::vector<uint8_t> binary(hex.size() / 2);
    size_t bin_len;
    
    if (sodium_hex2bin(
            binary.data(), binary.size(),
            hex.data(), hex.size(),
            nullptr, &bin_len, nullptr) != 0) {
        throw std::runtime_error("Invalid hex string");
    }
    
    binary.resize(bin_len);
    return binary;
}

// Base64 encoding
inline std::string to_base64(std::span<const uint8_t> data) {
    std::string b64(sodium_base64_ENCODED_LEN(
        data.size(), sodium_base64_VARIANT_ORIGINAL), '\0');
    
    sodium_bin2base64(
        b64.data(), b64.size(),
        data.data(), data.size(),
        sodium_base64_VARIANT_ORIGINAL
    );
    
    // Remove null terminator and padding
    b64.erase(std::find(b64.begin(), b64.end(), '\0'), b64.end());
    return b64;
}

// Generate random bytes
inline std::vector<uint8_t> random_bytes(size_t count) {
    std::vector<uint8_t> bytes(count);
    randombytes_buf(bytes.data(), bytes.size());
    return bytes;
}

// Secure zero memory
inline void secure_zero(std::span<uint8_t> data) noexcept {
    sodium_memzero(data.data(), data.size());
}

} // namespace util

} // namespace mithril::sodium

#endif // MITHRIL_SODIUM_HPP

// ============================================================================
// USAGE EXAMPLES
// ============================================================================
/*

// Example 1: Key Exchange
#include <mithril/mithril_sodium.hpp>
using namespace mithril::sodium;

KeyExchange alice, bob;
auto alice_shared = alice.derive_shared_secret(bob.get_public_key());
auto bob_shared = bob.derive_shared_secret(alice.get_public_key());
// alice_shared == bob_shared (ECDH success!)

// Example 2: Authenticated Encryption
AuthenticatedEncryption crypto(alice_shared);
auto ciphertext = crypto.encrypt(std::span{plaintext});
auto decrypted = crypto.decrypt(ciphertext);

// Example 3: Digital Signatures
DigitalSignature signer;
auto signature = signer.sign(message);
bool valid = signer.verify(message, signature, signer.get_verify_key());

// Example 4: Streaming Encryption
StreamEncryption stream;
auto header = stream.get_header();
auto chunk1 = stream.encrypt_chunk(data_chunk1, false);
auto chunk2 = stream.encrypt_chunk(data_chunk2, true); // final

// Example 5: Hashing
auto hash = CryptoHash::hash(data);
auto keyed_hash = CryptoHash::keyed_hash(data, key.span());

// Example 6: Utilities
auto hex = util::to_hex(data);
auto bytes = util::from_hex("deadbeef");
auto random = util::random_bytes(32);

*/
