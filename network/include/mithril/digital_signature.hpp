// ============================================================================
// Mithril Network Layer - Digital Signatures (Ed25519)
// ============================================================================
// File: network/include/mithril/digital_signature.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_DIGITAL_SIGNATURE_HPP
#define MITHRIL_DIGITAL_SIGNATURE_HPP

#include <mithril/common.hpp>

namespace mithril::sodium {

class DigitalSignature {
public:
    DigitalSignature();

    const VerifyKey &get_verify_key() const noexcept { return verify_key_; }

    Signature sign(std::span<const uint8_t> message) const;

    bool verify(
        std::span<const uint8_t> message,
        const Signature &signature,
        const VerifyKey &public_key) const noexcept;

    std::vector<uint8_t> sign_and_encrypt(
        std::span<const uint8_t> plaintext,
        const PublicKey &recipient_public_key,
        const SecretKey &sender_secret_key) const;

private:
    VerifyKey verify_key_;
    SigningKey signing_key_;
};

} // namespace mithril::sodium

#endif // MITHRIL_DIGITAL_SIGNATURE_HPP
