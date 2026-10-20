// ============================================================================
// Mithril Network Layer - Cryptographic Hashing
// ============================================================================
// File: network/include/mithril/crypto_hash.hpp
// ============================================================================

#pragma once

#ifndef MITHRIL_CRYPTO_HASH_HPP
#define MITHRIL_CRYPTO_HASH_HPP

#include <mithril/common.hpp>

namespace mithril::sodium {

class CryptoHash {
public:
    static Hash256 hash(std::span<const uint8_t> data);

    // Compatibility fallback while keyed-hash API is introduced in v2.
    static Hash256 keyed_hash(
        std::span<const uint8_t> data,
        std::span<const uint8_t> key);

    class MultiPartHash {
    public:
        MultiPartHash();

        void update(std::span<const uint8_t> data);

        Hash256 finalize();

    private:
        std::vector<uint8_t> buffer_;
    };
};

} // namespace mithril::sodium

#endif // MITHRIL_CRYPTO_HASH_HPP
