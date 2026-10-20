// ============================================================================
// Mithril Network Layer - v2 Crypto Wrapper (Umbrella Header)
// ============================================================================
// File: network/include/mithril/mithril_sodium.hpp
//
// Compatibility umbrella header that aggregates all modular components:
// - common: Types, runtime, concepts, secure memory, telemetry, utilities
// - key_exchange: X25519 ECDH key exchange
// - stream_encryption: AEAD authenticated & chunked stream encryption
// - digital_signature: Ed25519 digital signatures and signed encryption
// - crypto_hash: BLAKE2b cryptographic hashing and multi-part hash
// ============================================================================

#pragma once

#ifndef MITHRIL_SODIUM_HPP
#define MITHRIL_SODIUM_HPP

#include <mithril/common.hpp>
#include <mithril/crypto_hash.hpp>
#include <mithril/digital_signature.hpp>
#include <mithril/key_exchange.hpp>
#include <mithril/stream_encryption.hpp>

#endif // MITHRIL_SODIUM_HPP
