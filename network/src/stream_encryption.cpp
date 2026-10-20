// ============================================================================
// Mithril Network Layer - Stream & Authenticated Encryption Implementation
// ============================================================================
// File: network/src/stream_encryption.cpp
// ============================================================================

#include <mithril/stream_encryption.hpp>
#include <mithril/mithril_aead.h>

namespace mithril::sodium {

// ============================================================================
// AuthenticatedEncryption Implementation
// ============================================================================

AuthenticatedEncryption::AuthenticatedEncryption(const SessionKey &key) {
    SodiumInit::instance();
    std::copy_n(key.data(), key.size(), key_.data());
}

AuthenticatedEncryption::AuthenticatedEncryption() {
    SodiumInit::instance();
    key_.randomize();
}

std::vector<uint8_t> AuthenticatedEncryption::encrypt(std::span<const uint8_t> plaintext) const {
    Nonce nonce{};
    size_t written_len = 0u;

    detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), nonce.data(), nonce.size()));

    std::vector<uint8_t> cipher(plaintext.size() + MITHRIL_AEAD_TAG_BYTES);
    static const uint8_t kEmptyDummy = 0u;
    const uint8_t *pt_ptr = plaintext.data() ? plaintext.data() : &kEmptyDummy;

    detail::require_ok(
        "mithril_aead_encrypt",
        mithril_aead_encrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            key_.data(),
            key_.size(),
            nonce.data(),
            nonce.size(),
            nullptr,
            0u,
            pt_ptr,
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

std::vector<uint8_t> AuthenticatedEncryption::decrypt(std::span<const uint8_t> ciphertext) const {
    if (ciphertext.size() < Nonce{}.size() + MITHRIL_AEAD_TAG_BYTES) {
        throw std::invalid_argument("Ciphertext too short");
    }

    Nonce nonce{};
    std::copy_n(ciphertext.begin(), nonce.size(), nonce.begin());

    std::span<const uint8_t> cipher_data = ciphertext.subspan(nonce.size());
    std::vector<uint8_t> plaintext(cipher_data.size());
    size_t written_len = 0u;
    static uint8_t kEmptyOutDummy = 0u;
    uint8_t *pt_ptr = plaintext.data() ? plaintext.data() : &kEmptyOutDummy;

    detail::require_ok(
        "mithril_aead_decrypt",
        mithril_aead_decrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            key_.data(),
            key_.size(),
            nonce.data(),
            nonce.size(),
            nullptr,
            0u,
            cipher_data.data(),
            cipher_data.size(),
            pt_ptr,
            plaintext.size(),
            &written_len));

    plaintext.resize(written_len);
    return plaintext;
}

// ============================================================================
// StreamEncryption Implementation
// ============================================================================

StreamEncryption::StreamEncryption() {
    SodiumInit::instance();
    key_.randomize();
    init_push_state();
}

StreamEncryption::StreamEncryption(const SymmetricKey &key) {
    SodiumInit::instance();
    std::copy_n(key.data(), key.size(), key_.data());
    init_push_state();
}

std::vector<uint8_t> StreamEncryption::encrypt_chunk(
    std::span<const uint8_t> chunk,
    bool is_final) {

    Nonce nonce = make_nonce(header_, push_counter_++);
    const uint8_t flag = static_cast<uint8_t>(is_final ? 1u : 0u);

    std::vector<uint8_t> cipher(chunk.size() + MITHRIL_AEAD_TAG_BYTES);
    size_t written_len = 0u;
    static const uint8_t kEmptyDummy = 0u;
    const uint8_t *chunk_ptr = chunk.data() ? chunk.data() : &kEmptyDummy;

    detail::require_ok(
        "mithril_aead_encrypt",
        mithril_aead_encrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            key_.data(),
            key_.size(),
            nonce.data(),
            nonce.size(),
            &flag,
            1u,
            chunk_ptr,
            chunk.size(),
            cipher.data(),
            cipher.size(),
            &written_len));

    cipher.resize(written_len);

    std::vector<uint8_t> framed;
    framed.reserve(1u + cipher.size());
    framed.push_back(flag);
    framed.insert(framed.end(), cipher.begin(), cipher.end());
    return framed;
}

void StreamEncryption::init_pull_state(const StreamHeader &header) {
    pull_header_ = header;
    pull_counter_ = 0u;
    pull_initialized_ = true;
}

std::pair<std::vector<uint8_t>, bool> StreamEncryption::decrypt_chunk(
    std::span<const uint8_t> ciphertext) {

    if (!pull_initialized_) {
        throw std::runtime_error("Pull state not initialized");
    }
    if (ciphertext.size() < 1u + MITHRIL_AEAD_TAG_BYTES) {
        throw std::invalid_argument("Ciphertext chunk too short");
    }

    const uint8_t flag = ciphertext[0];
    std::span<const uint8_t> cipher = ciphertext.subspan(1u);
    Nonce nonce = make_nonce(pull_header_, pull_counter_++);

    std::vector<uint8_t> plaintext(cipher.size());
    size_t written_len = 0u;
    static uint8_t kEmptyOutDummy = 0u;
    uint8_t *pt_ptr = plaintext.data() ? plaintext.data() : &kEmptyOutDummy;

    detail::require_ok(
        "mithril_aead_decrypt",
        mithril_aead_decrypt(
            detail::context(),
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            key_.data(),
            key_.size(),
            nonce.data(),
            nonce.size(),
            &flag,
            1u,
            cipher.data(),
            cipher.size(),
            pt_ptr,
            plaintext.size(),
            &written_len));

    plaintext.resize(written_len);
    const bool is_final = (flag == 1u);
    return {std::move(plaintext), is_final};
}

Nonce StreamEncryption::make_nonce(const StreamHeader &header, uint64_t counter) {
    Nonce nonce{};
    std::copy(header.begin(), header.end(), nonce.begin());

    for (size_t i = 0u; i < sizeof(counter); ++i) {
        const size_t idx = nonce.size() - sizeof(counter) + i;
        nonce[idx] = static_cast<uint8_t>((counter >> ((sizeof(counter) - 1u - i) * 8u)) & 0xFFu);
    }

    return nonce;
}

void StreamEncryption::init_push_state() {
    detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), header_.data(), header_.size()));
    pull_header_ = header_;
    pull_initialized_ = true;
    push_counter_ = 0u;
    pull_counter_ = 0u;
}

} // namespace mithril::sodium
