// ============================================================================
// Mithril Network Layer - v2 Crypto Wrapper
// ============================================================================
// File: network/include/mithril/mithril_sodium.hpp
//
// Compatibility wrapper that preserves the previous C++ surface while routing
// all core cryptographic operations through Mithril API v2.
// ============================================================================

#pragma once

#ifndef MITHRIL_SODIUM_HPP
#define MITHRIL_SODIUM_HPP

#include <algorithm>
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <mithril/mithril_aead.h>
#include <mithril/mithril_api.h>
#include <mithril/mithril_hash.h>
#include <mithril/mithril_kex.h>
#include <mithril/mithril_provider.h>
#include <mithril/mithril_rng.h>
#include <mithril/mithril_sign.h>

namespace mithril::sodium {

inline constexpr size_t MITHRIL_PUBLIC_KEY_BYTES = 32u;
inline constexpr size_t MITHRIL_SECRET_KEY_BYTES = 32u;
inline constexpr size_t MITHRIL_SESSION_KEY_BYTES = 32u;
inline constexpr size_t MITHRIL_SIGN_SECRET_KEY_BYTES = 64u;
inline constexpr size_t MITHRIL_SIGN_PUBLIC_KEY_BYTES = 32u;
inline constexpr size_t MITHRIL_SIGN_SIGNATURE_BYTES = 64u;
inline constexpr size_t MITHRIL_AEAD_NONCE_BYTES = 24u;
inline constexpr size_t MITHRIL_AEAD_TAG_BYTES = 16u;
inline constexpr size_t MITHRIL_HASH_BYTES = 32u;
inline constexpr size_t MITHRIL_STREAM_HEADER_BYTES = 24u;

namespace telemetry {

struct Counters {
    uint64_t total_operations;
    uint64_t total_failures;
    uint64_t verification_failures;
    uint64_t internal_failures;
};

using FailureHook = void (*)(
    const char *operation,
    mithril_status status,
    const char *status_string,
    void *user_data);

void set_failure_hook(FailureHook hook, void *user_data = nullptr) noexcept;
void clear_failure_hook() noexcept;
void reset() noexcept;
Counters snapshot() noexcept;
double failure_rate() noexcept;

} // namespace telemetry

namespace detail {

struct TelemetryState {
    std::atomic<uint64_t> total_operations{0u};
    std::atomic<uint64_t> total_failures{0u};
    std::atomic<uint64_t> verification_failures{0u};
    std::atomic<uint64_t> internal_failures{0u};
    std::atomic<telemetry::FailureHook> failure_hook{nullptr};
    std::atomic<void *> failure_hook_user_data{nullptr};
};

inline TelemetryState &telemetry_state() noexcept {
    static TelemetryState state;
    return state;
}

inline void secure_memzero(void *ptr, size_t len) noexcept {
    volatile uint8_t *p = static_cast<volatile uint8_t *>(ptr);
    while (len-- > 0u) {
        *p++ = 0u;
    }
}

inline bool ct_equal(const uint8_t *a, const uint8_t *b, size_t len) noexcept {
    uint8_t diff = 0u;
    for (size_t i = 0u; i < len; ++i) {
        diff |= static_cast<uint8_t>(a[i] ^ b[i]);
    }
    return diff == 0u;
}

inline bool telemetry_stderr_enabled() noexcept {
    const char *raw = std::getenv("MITHRIL_CRYPTO_TELEMETRY_STDERR");
    if (raw == nullptr || raw[0] == '\0') {
        return false;
    }
    return raw[0] == '1' || raw[0] == 'y' || raw[0] == 'Y' || raw[0] == 't' || raw[0] == 'T';
}

inline void record_status(const char *operation, mithril_status status) noexcept {
    TelemetryState &state = telemetry_state();
    state.total_operations.fetch_add(1u, std::memory_order_relaxed);

    if (status == MITHRIL_OK) {
        return;
    }

    state.total_failures.fetch_add(1u, std::memory_order_relaxed);

    if (status == MITHRIL_ERR_AEAD_AUTH_FAILED || status == MITHRIL_ERR_SIGNATURE_INVALID) {
        state.verification_failures.fetch_add(1u, std::memory_order_relaxed);
    } else {
        state.internal_failures.fetch_add(1u, std::memory_order_relaxed);
    }

    if (telemetry_stderr_enabled()) {
        std::fprintf(
            stderr,
            "[mithril-telemetry] op=%s status=%s\n",
            operation,
            mithril_status_string(status));
    }

    const telemetry::FailureHook hook = state.failure_hook.load(std::memory_order_acquire);
    if (hook != nullptr) {
        void *user_data = state.failure_hook_user_data.load(std::memory_order_acquire);
        try {
            hook(operation, status, mithril_status_string(status), user_data);
        } catch (...) {
            // Telemetry hooks must never interfere with crypto flow.
        }
    }
}

[[noreturn]] inline void throw_status(const char *operation, mithril_status status) {
    std::string msg = operation;
    msg += " failed: ";
    msg += mithril_status_string(status);
    throw std::runtime_error(msg);
}

inline void require_ok(const char *operation, mithril_status status) {
    record_status(operation, status);
    if (status != MITHRIL_OK) {
        throw_status(operation, status);
    }
}

class Runtime {
public:
    Runtime() {
        mithril_status status = mithril_init(&ctx_, nullptr);
        if (status != MITHRIL_OK || ctx_ == nullptr) {
            throw_status("mithril_init", status);
        }

        // Keep explicit provider selection for deterministic behavior in
        // consumer migration.
        status = mithril_provider_activate(ctx_, "sodium");
        if (status != MITHRIL_OK) {
            mithril_shutdown(ctx_);
            ctx_ = nullptr;
            throw_status("mithril_provider_activate(sodium)", status);
        }
    }

    ~Runtime() {
        if (ctx_ != nullptr) {
            mithril_shutdown(ctx_);
            ctx_ = nullptr;
        }
    }

    Runtime(const Runtime &) = delete;
    Runtime &operator=(const Runtime &) = delete;
    Runtime(Runtime &&) = delete;
    Runtime &operator=(Runtime &&) = delete;

    mithril_context *context() const noexcept { return ctx_; }

private:
    mithril_context *ctx_ = nullptr;
};

inline Runtime &runtime() {
    static Runtime instance;
    return instance;
}

inline mithril_context *context() {
    return runtime().context();
}

inline uint8_t hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>(10 + (c - 'a'));
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>(10 + (c - 'A'));
    }
    throw std::runtime_error("Invalid hex character");
}

} // namespace detail

namespace telemetry {

inline void set_failure_hook(FailureHook hook, void *user_data) noexcept {
    detail::TelemetryState &state = detail::telemetry_state();
    state.failure_hook_user_data.store(user_data, std::memory_order_release);
    state.failure_hook.store(hook, std::memory_order_release);
}

inline void clear_failure_hook() noexcept {
    set_failure_hook(nullptr, nullptr);
}

inline void reset() noexcept {
    detail::TelemetryState &state = detail::telemetry_state();
    state.total_operations.store(0u, std::memory_order_relaxed);
    state.total_failures.store(0u, std::memory_order_relaxed);
    state.verification_failures.store(0u, std::memory_order_relaxed);
    state.internal_failures.store(0u, std::memory_order_relaxed);
}

inline Counters snapshot() noexcept {
    const detail::TelemetryState &state = detail::telemetry_state();
    return Counters{
        .total_operations = state.total_operations.load(std::memory_order_relaxed),
        .total_failures = state.total_failures.load(std::memory_order_relaxed),
        .verification_failures = state.verification_failures.load(std::memory_order_relaxed),
        .internal_failures = state.internal_failures.load(std::memory_order_relaxed),
    };
}

inline double failure_rate() noexcept {
    const Counters counters = snapshot();
    if (counters.total_operations == 0u) {
        return 0.0;
    }
    return static_cast<double>(counters.total_failures) / static_cast<double>(counters.total_operations);
}

} // namespace telemetry

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

using PublicKey = SecureKey<MITHRIL_PUBLIC_KEY_BYTES>;
using SecretKey = SecureKey<MITHRIL_SECRET_KEY_BYTES>;
using SessionKey = SecureKey<MITHRIL_SESSION_KEY_BYTES>;
using SigningKey = SecureKey<MITHRIL_SIGN_SECRET_KEY_BYTES>;
using VerifyKey = SecureKey<MITHRIL_SIGN_PUBLIC_KEY_BYTES>;
using SymmetricKey = SecureKey<MITHRIL_SESSION_KEY_BYTES>;

using Nonce = std::array<uint8_t, MITHRIL_AEAD_NONCE_BYTES>;
using StreamHeader = std::array<uint8_t, MITHRIL_STREAM_HEADER_BYTES>;

using Hash256 = std::array<uint8_t, MITHRIL_HASH_BYTES>;
using Signature = std::array<uint8_t, MITHRIL_SIGN_SIGNATURE_BYTES>;

// ============================================================================
// Concepts for Type Safety
// ============================================================================

template<typename T>
concept SecureKeyType = requires(T key) {
    { key.data() } -> std::convertible_to<uint8_t *>;
    { key.size() } -> std::convertible_to<size_t>;
    { key.span() } -> std::convertible_to<std::span<uint8_t>>;
};

template<typename T>
concept ByteSpan = std::convertible_to<T, std::span<const uint8_t>>;

// ============================================================================
// Global Runtime Initialization (RAII)
// ============================================================================

class SodiumInit {
public:
    SodiumInit() { (void)detail::runtime(); }

    static SodiumInit &instance() {
        static SodiumInit init;
        return init;
    }

    SodiumInit(const SodiumInit &) = delete;
    SodiumInit &operator=(const SodiumInit &) = delete;
    SodiumInit(SodiumInit &&) = delete;
    SodiumInit &operator=(SodiumInit &&) = delete;
};

// ============================================================================
// Secure Key Template
// ============================================================================

template<size_t N>
class SecureKey {
public:
    SecureKey() = default;

    ~SecureKey() { detail::secure_memzero(data_.data(), data_.size()); }

    SecureKey(const SecureKey &) = delete;
    SecureKey &operator=(const SecureKey &) = delete;

    SecureKey(SecureKey &&other) noexcept {
        std::copy(other.data_.begin(), other.data_.end(), data_.begin());
        detail::secure_memzero(other.data_.data(), other.data_.size());
    }

    SecureKey &operator=(SecureKey &&other) noexcept {
        if (this != &other) {
            std::copy(other.data_.begin(), other.data_.end(), data_.begin());
            detail::secure_memzero(other.data_.data(), other.data_.size());
        }
        return *this;
    }

    uint8_t *data() noexcept { return data_.data(); }
    const uint8_t *data() const noexcept { return data_.data(); }
    constexpr size_t size() const noexcept { return N; }

    std::span<uint8_t> span() noexcept { return {data_.data(), N}; }
    std::span<const uint8_t> span() const noexcept { return {data_.data(), N}; }

    void randomize() {
        detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), data_.data(), data_.size()));
    }

    bool operator==(const SecureKey &other) const noexcept {
        return detail::ct_equal(data_.data(), other.data_.data(), N);
    }

    bool operator!=(const SecureKey &other) const noexcept {
        return !(*this == other);
    }

private:
    std::array<uint8_t, N> data_{};
};

// ============================================================================
// Key Exchange (X25519 via API v2)
// ============================================================================

class KeyExchange {
public:
    KeyExchange() {
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

    const PublicKey &get_public_key() const noexcept { return public_key_; }

    SessionKey derive_shared_secret(const PublicKey &peer_public_key) const {
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

    std::vector<uint8_t> encrypt_to(
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

    std::vector<uint8_t> decrypt_from(
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

private:
    PublicKey public_key_;
    SecretKey secret_key_;
};

// ============================================================================
// Authenticated Encryption (AEAD via API v2)
// ============================================================================

class AuthenticatedEncryption {
public:
    explicit AuthenticatedEncryption(const SessionKey &key) {
        SodiumInit::instance();
        std::copy_n(key.data(), key.size(), key_.data());
    }

    AuthenticatedEncryption() {
        SodiumInit::instance();
        key_.randomize();
    }

    std::vector<uint8_t> encrypt(std::span<const uint8_t> plaintext) const {
        Nonce nonce{};
        size_t written_len = 0u;

        detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), nonce.data(), nonce.size()));

        std::vector<uint8_t> cipher(plaintext.size() + MITHRIL_AEAD_TAG_BYTES);
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

    std::vector<uint8_t> decrypt(std::span<const uint8_t> ciphertext) const {
        if (ciphertext.size() < Nonce{}.size() + MITHRIL_AEAD_TAG_BYTES) {
            throw std::invalid_argument("Ciphertext too short");
        }

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
                key_.data(),
                key_.size(),
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

    const SessionKey &get_key() const noexcept { return key_; }

private:
    SessionKey key_;
};

// ============================================================================
// Stream Encryption (chunk framing over AEAD)
// ============================================================================

class StreamEncryption {
public:
    StreamEncryption() {
        SodiumInit::instance();
        key_.randomize();
        init_push_state();
    }

    explicit StreamEncryption(const SymmetricKey &key) {
        SodiumInit::instance();
        std::copy_n(key.data(), key.size(), key_.data());
        init_push_state();
    }

    const StreamHeader &get_header() const noexcept { return header_; }

    std::vector<uint8_t> encrypt_chunk(
        std::span<const uint8_t> chunk,
        bool is_final = false) {

        Nonce nonce = make_nonce(header_, push_counter_++);
        const uint8_t flag = static_cast<uint8_t>(is_final ? 1u : 0u);

        std::vector<uint8_t> cipher(chunk.size() + MITHRIL_AEAD_TAG_BYTES);
        size_t written_len = 0u;
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
                chunk.data(),
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

    void init_pull_state(const StreamHeader &header) {
        pull_header_ = header;
        pull_counter_ = 0u;
        pull_initialized_ = true;
    }

    std::pair<std::vector<uint8_t>, bool> decrypt_chunk(
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
                plaintext.data(),
                plaintext.size(),
                &written_len));

        plaintext.resize(written_len);
        const bool is_final = (flag == 1u);
        return {std::move(plaintext), is_final};
    }

private:
    static Nonce make_nonce(const StreamHeader &header, uint64_t counter) {
        Nonce nonce{};
        std::copy(header.begin(), header.end(), nonce.begin());

        for (size_t i = 0u; i < sizeof(counter); ++i) {
            const size_t idx = nonce.size() - sizeof(counter) + i;
            nonce[idx] = static_cast<uint8_t>((counter >> ((sizeof(counter) - 1u - i) * 8u)) & 0xFFu);
        }

        return nonce;
    }

    void init_push_state() {
        detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), header_.data(), header_.size()));
        pull_header_ = header_;
        pull_initialized_ = true;
        push_counter_ = 0u;
        pull_counter_ = 0u;
    }

    SymmetricKey key_;
    StreamHeader header_{};
    StreamHeader pull_header_{};
    uint64_t push_counter_ = 0u;
    uint64_t pull_counter_ = 0u;
    bool pull_initialized_ = false;
};

// ============================================================================
// Digital Signatures (Ed25519 via API v2)
// ============================================================================

class DigitalSignature {
public:
    DigitalSignature() {
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

    const VerifyKey &get_verify_key() const noexcept { return verify_key_; }

    Signature sign(std::span<const uint8_t> message) const {
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

    bool verify(
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

    std::vector<uint8_t> sign_and_encrypt(
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

private:
    VerifyKey verify_key_;
    SigningKey signing_key_;
};

// ============================================================================
// Cryptographic Hashing
// ============================================================================

class CryptoHash {
public:
    static Hash256 hash(std::span<const uint8_t> data) {
        SodiumInit::instance();

        Hash256 out{};
        size_t written_len = 0u;
        detail::require_ok(
            "mithril_hash_compute",
            mithril_hash_compute(
                detail::context(),
                MITHRIL_HASH_ALG_BLAKE2B_256,
                data.data(),
                data.size(),
                out.data(),
                out.size(),
                &written_len));

        if (written_len != out.size()) {
            throw std::runtime_error("mithril_hash_compute returned unexpected digest size");
        }

        return out;
    }

    // Compatibility fallback while keyed-hash API is introduced in v2.
    static Hash256 keyed_hash(
        std::span<const uint8_t> data,
        std::span<const uint8_t> key) {

        std::vector<uint8_t> combined;
        combined.reserve(key.size() + data.size());
        combined.insert(combined.end(), key.begin(), key.end());
        combined.insert(combined.end(), data.begin(), data.end());
        return hash(combined);
    }

    class MultiPartHash {
    public:
        MultiPartHash() { SodiumInit::instance(); }

        void update(std::span<const uint8_t> data) {
            buffer_.insert(buffer_.end(), data.begin(), data.end());
        }

        Hash256 finalize() {
            return CryptoHash::hash(buffer_);
        }

    private:
        std::vector<uint8_t> buffer_;
    };
};

// ============================================================================
// Utility Functions
// ============================================================================

namespace util {

inline bool secure_compare(
    std::span<const uint8_t> a,
    std::span<const uint8_t> b) noexcept {

    if (a.size() != b.size()) {
        return false;
    }

    return detail::ct_equal(a.data(), b.data(), a.size());
}

inline std::string to_hex(std::span<const uint8_t> data) {
    static constexpr char kHex[] = "0123456789abcdef";

    std::string out;
    out.resize(data.size() * 2u);

    for (size_t i = 0u; i < data.size(); ++i) {
        out[2u * i] = kHex[(data[i] >> 4u) & 0x0Fu];
        out[2u * i + 1u] = kHex[data[i] & 0x0Fu];
    }

    return out;
}

inline std::vector<uint8_t> from_hex(std::string_view hex) {
    if ((hex.size() % 2u) != 0u) {
        throw std::runtime_error("Hex string must have even length");
    }

    std::vector<uint8_t> out(hex.size() / 2u);

    for (size_t i = 0u; i < out.size(); ++i) {
        const uint8_t hi = detail::hex_nibble(hex[2u * i]);
        const uint8_t lo = detail::hex_nibble(hex[2u * i + 1u]);
        out[i] = static_cast<uint8_t>((hi << 4u) | lo);
    }

    return out;
}

inline std::string to_base64(std::span<const uint8_t> data) {
    static constexpr char kTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve(((data.size() + 2u) / 3u) * 4u);

    size_t i = 0u;
    while (i + 3u <= data.size()) {
        const uint32_t n = (static_cast<uint32_t>(data[i]) << 16u) |
                           (static_cast<uint32_t>(data[i + 1u]) << 8u) |
                           static_cast<uint32_t>(data[i + 2u]);

        out.push_back(kTable[(n >> 18u) & 0x3Fu]);
        out.push_back(kTable[(n >> 12u) & 0x3Fu]);
        out.push_back(kTable[(n >> 6u) & 0x3Fu]);
        out.push_back(kTable[n & 0x3Fu]);
        i += 3u;
    }

    const size_t rem = data.size() - i;
    if (rem == 1u) {
        const uint32_t n = static_cast<uint32_t>(data[i]) << 16u;
        out.push_back(kTable[(n >> 18u) & 0x3Fu]);
        out.push_back(kTable[(n >> 12u) & 0x3Fu]);
        out.push_back('=');
        out.push_back('=');
    } else if (rem == 2u) {
        const uint32_t n = (static_cast<uint32_t>(data[i]) << 16u) |
                           (static_cast<uint32_t>(data[i + 1u]) << 8u);
        out.push_back(kTable[(n >> 18u) & 0x3Fu]);
        out.push_back(kTable[(n >> 12u) & 0x3Fu]);
        out.push_back(kTable[(n >> 6u) & 0x3Fu]);
        out.push_back('=');
    }

    return out;
}

inline std::vector<uint8_t> random_bytes(size_t count) {
    std::vector<uint8_t> bytes(count);
    if (count == 0u) {
        return bytes;
    }

    detail::require_ok("mithril_rng_fill", mithril_rng_fill(detail::context(), bytes.data(), bytes.size()));
    return bytes;
}

inline void secure_zero(std::span<uint8_t> data) noexcept {
    detail::secure_memzero(data.data(), data.size());
}

} // namespace util

} // namespace mithril::sodium

#endif // MITHRIL_SODIUM_HPP
