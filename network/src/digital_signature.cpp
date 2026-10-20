// ============================================================================
// Mithril Network Layer - Digital Signature Implementation
// ============================================================================
// File: network/src/digital_signature.cpp
// ============================================================================

#include <mithril/digital_signature.hpp>
#include <mithril/mithril_aead.h>
#include <mithril/mithril_kex.h>
#include <mithril/mithril_sign.h>

namespace mithril::sodium {

DigitalSignature::DigitalSignature() {
    SodiumInit::instance();
    detail::require_ok(
        "mithril_sign_keypair",
        mithril_sign_keypair(
            detail::context(),
            MITHRIL_SIGN_ALG_ED25519,
            verify_key_.data(),
            verify_key_.size(),
            signing_key_.data(),
            signing_key_.size()));
}

Signature DigitalSignature::sign(std::span<const uint8_t> message) const {
    Signature signature{};
    size_t written_len = 0u;

    detail::require_ok(
        "mithril_sign_detached",
        mithril_sign_detached(
            detail::context(),
            MITHRIL_SIGN_ALG_ED25519,
            message.data(),
            message.size(),
            signing_key_.data(),
            signing_key_.size(),
            signature.data(),
            signature.size(),
            &written_len));

    if (written_len != signature.size()) {
        throw std::runtime_error("mithril_sign_detached returned unexpected signature size");
    }

    return signature;
}

bool DigitalSignature::verify(
    std::span<const uint8_t> message,
    const Signature &signature,
    const VerifyKey &public_key) const noexcept {

    const mithril_status st = mithril_sign_verify_detached(
        detail::context(),
        MITHRIL_SIGN_ALG_ED25519,
        message.data(),
        message.size(),
        public_key.data(),
        public_key.size(),
        signature.data(),
        signature.size());

    detail::record_status("mithril_sign_verify_detached", st);
    return st == MITHRIL_OK;
}

std::vector<uint8_t> DigitalSignature::sign_and_encrypt(
    std::span<const uint8_t> plaintext,
    const PublicKey &recipient_public_key,
    const SecretKey &sender_secret_key) const {

    auto signature = sign(plaintext);

    std::vector<uint8_t> signed_data;
    signed_data.reserve(signature.size() + plaintext.size());
    signed_data.insert(signed_data.end(), signature.begin(), signature.end());
    signed_data.insert(signed_data.end(), plaintext.begin(), plaintext.end());

    SessionKey shared;
    size_t shared_len = 0u;
    detail::require_ok(
        "mithril_kex_shared_secret",
        mithril_kex_shared_secret(
            detail::context(),
            MITHRIL_KEX_ALG_X25519,
            sender_secret_key.data(),
            sender_secret_key.size(),
            recipient_public_key.data(),
            recipient_public_key.size(),
            shared.data(),
            shared.size(),
            &shared_len));

    if (shared_len != shared.size()) {
        throw std::runtime_error("mithril_kex_shared_secret returned unexpected size");
    }

    Nonce nonce{};
    detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), nonce.data(), nonce.size()));

    std::vector<uint8_t> cipher(signed_data.size() + MITHRIL_AEAD_TAG_BYTES);
    size_t written_len = 0u;
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
            signed_data.data(),
            signed_data.size(),
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

} // namespace mithril::sodium
