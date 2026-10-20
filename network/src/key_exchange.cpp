// ============================================================================
// Mithril Network Layer - Key Exchange Implementation
// ============================================================================
// File: network/src/key_exchange.cpp
// ============================================================================

#include <mithril/key_exchange.hpp>
#include <mithril/mithril_aead.h>
#include <mithril/mithril_kex.h>

namespace mithril::sodium {

KeyExchange::KeyExchange() {
    SodiumInit::instance();
    detail::require_ok(
        "mithril_kex_keypair",
        mithril_kex_keypair(
            detail::context(),
            MITHRIL_KEX_ALG_X25519,
            public_key_.data(),
            public_key_.size(),
            secret_key_.data(),
            secret_key_.size()));
}

SessionKey KeyExchange::derive_shared_secret(const PublicKey &peer_public_key) const {
    SessionKey shared;
    size_t written_len = 0u;

    detail::require_ok(
        "mithril_kex_shared_secret",
        mithril_kex_shared_secret(
            detail::context(),
            MITHRIL_KEX_ALG_X25519,
            secret_key_.data(),
            secret_key_.size(),
            peer_public_key.data(),
            peer_public_key.size(),
            shared.data(),
            shared.size(),
            &written_len));

    if (written_len != shared.size()) {
        throw std::runtime_error("mithril_kex_shared_secret returned unexpected size");
    }

    return shared;
}

std::vector<uint8_t> KeyExchange::encrypt_to(
    std::span<const uint8_t> plaintext,
    const PublicKey &recipient_public_key) const {

    SessionKey shared = derive_shared_secret(recipient_public_key);
    Nonce nonce{};
    size_t written_len = 0u;

    detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), nonce.data(), nonce.size()));

    std::vector<uint8_t> cipher(plaintext.size() + MITHRIL_AEAD_TAG_BYTES);
    detail::require_ok(
        "mithril_aead_encrypt",
        mithril_aead_encrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            shared.data(),
            shared.size(),
            nonce.data(),
            nonce.size(),
            nullptr,
            0u,
            plaintext.data(),
            plaintext.size(),
            cipher.data(),
            cipher.size(),
            &written_len));

    cipher.resize(written_len);

    std::vector<uint8_t> out;
    out.reserve(nonce.size() + cipher.size());
    out.insert(out.end(), nonce.begin(), nonce.end());
    out.insert(out.end(), cipher.begin(), cipher.end());
    return out;
}

std::vector<uint8_t> KeyExchange::decrypt_from(
    std::span<const uint8_t> ciphertext,
    const PublicKey &sender_public_key) const {

    if (ciphertext.size() < Nonce{}.size() + MITHRIL_AEAD_TAG_BYTES) {
        throw std::invalid_argument("Ciphertext too short");
    }

    SessionKey shared = derive_shared_secret(sender_public_key);

    Nonce nonce{};
    std::copy_n(ciphertext.begin(), nonce.size(), nonce.begin());

    std::span<const uint8_t> cipher_data = ciphertext.subspan(nonce.size());
    std::vector<uint8_t> plaintext(cipher_data.size());
    size_t written_len = 0u;

    detail::require_ok(
        "mithril_aead_decrypt",
        mithril_aead_decrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            shared.data(),
            shared.size(),
            nonce.data(),
            nonce.size(),
            nullptr,
            0u,
            cipher_data.data(),
            cipher_data.size(),
            plaintext.data(),
            plaintext.size(),
            &written_len));

    plaintext.resize(written_len);
    return plaintext;
}

} // namespace mithril::sodium
