// ============================================================================
// Mithril Network Layer - Cryptographic Hashing Implementation
// ============================================================================
// File: network/src/crypto_hash.cpp
// ============================================================================

#include <mithril/crypto_hash.hpp>
#include <mithril/mithril_hash.h>

namespace mithril::sodium {

Hash256 CryptoHash::hash(std::span<const uint8_t> data) {
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

Hash256 CryptoHash::keyed_hash(
    std::span<const uint8_t> data,
    std::span<const uint8_t> key) {

    std::vector<uint8_t> combined;
    combined.reserve(key.size() + data.size());
    combined.insert(combined.end(), key.begin(), key.end());
    combined.insert(combined.end(), data.begin(), data.end());
    return hash(combined);
}

CryptoHash::MultiPartHash::MultiPartHash() {
    SodiumInit::instance();
}

void CryptoHash::MultiPartHash::update(std::span<const uint8_t> data) {
    buffer_.insert(buffer_.end(), data.begin(), data.end());
}

Hash256 CryptoHash::MultiPartHash::finalize() {
    return CryptoHash::hash(buffer_);
}

} // namespace mithril::sodium
