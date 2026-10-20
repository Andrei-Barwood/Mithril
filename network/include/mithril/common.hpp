// ============================================================================
// Mithril Network Layer - Common Types, Runtime & Telemetry
// ============================================================================
// File: network/include/mithril/common.hpp
//
// Core foundations, secure memory management, concepts, RAII runtime,
// and telemetry hooks for Mithril C++20 wrappers over Mithril API v2.
// ============================================================================

#pragma once

#ifndef MITHRIL_COMMON_HPP
#define MITHRIL_COMMON_HPP

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

#include <mithril/mithril_api.h>
#include <mithril/mithril_provider.h>
#include <mithril/mithril_rng.h>

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

// ============================================================================
// Telemetry
// ============================================================================

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

// ============================================================================
// Detail (Internal Implementation Helpers)
// ============================================================================

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

#endif // MITHRIL_COMMON_HPP
