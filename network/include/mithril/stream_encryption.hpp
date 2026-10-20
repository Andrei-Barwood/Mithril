// ============================================================================
// Mithril Network Layer - Authenticated & Stream Encryption
// ============================================================================
// File: network/include/mithril/stream_encryption.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_STREAM_ENCRYPTION_HPP
#define MITHRIL_STREAM_ENCRYPTION_HPP

#include <mithril/common.hpp>

namespace mithril::sodium {

// ============================================================================
// Authenticated Encryption (AEAD via API v2)
// ============================================================================

class AuthenticatedEncryption {
public:
    explicit AuthenticatedEncryption(const SessionKey &key);
    AuthenticatedEncryption();

    std::vector<uint8_t> encrypt(std::span<const uint8_t> plaintext) const;
    std::vector<uint8_t> decrypt(std::span<const uint8_t> ciphertext) const;

    const SessionKey &get_key() const noexcept { return key_; }

private:
    SessionKey key_;
};

// ============================================================================
// Stream Encryption (chunk framing over AEAD)
// ============================================================================

class StreamEncryption {
public:
    StreamEncryption();
    explicit StreamEncryption(const SymmetricKey &key);

    const StreamHeader &get_header() const noexcept { return header_; }
    const SymmetricKey &get_key() const noexcept { return key_; }

    std::vector<uint8_t> encrypt_chunk(
        std::span<const uint8_t> chunk,
        bool is_final = false);

    void init_pull_state(const StreamHeader &header);

    std::pair<std::vector<uint8_t>, bool> decrypt_chunk(
        std::span<const uint8_t> ciphertext);

private:
    static Nonce make_nonce(const StreamHeader &header, uint64_t counter);
    void init_push_state();

    SymmetricKey key_;
    StreamHeader header_{};
    StreamHeader pull_header_{};
    uint64_t push_counter_ = 0u;
    uint64_t pull_counter_ = 0u;
    bool pull_initialized_ = false;
};

} // namespace mithril::sodium

#endif // MITHRIL_STREAM_ENCRYPTION_HPP
