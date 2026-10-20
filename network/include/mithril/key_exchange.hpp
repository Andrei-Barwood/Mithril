// ============================================================================
// Mithril Network Layer - Key Exchange (X25519)
// ============================================================================
// File: network/include/mithril/key_exchange.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_KEY_EXCHANGE_HPP
#define MITHRIL_KEY_EXCHANGE_HPP

#include <mithril/common.hpp>

namespace mithril::sodium {

class KeyExchange {
public:
    KeyExchange();

    const PublicKey &get_public_key() const noexcept { return public_key_; }

    SessionKey derive_shared_secret(const PublicKey &peer_public_key) const;

    std::vector<uint8_t> encrypt_to(
        std::span<const uint8_t> plaintext,
        const PublicKey &recipient_public_key) const;

    std::vector<uint8_t> decrypt_from(
        std::span<const uint8_t> ciphertext,
        const PublicKey &sender_public_key) const;

private:
    PublicKey public_key_;
    SecretKey secret_key_;
};

} // namespace mithril::sodium

#endif // MITHRIL_KEY_EXCHANGE_HPP
