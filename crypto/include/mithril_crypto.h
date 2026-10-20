// ============================================================================
// Mithril Cryptography Layer - Transitional Forwarding Header
// File: crypto/include/mithril_crypto.h
// ============================================================================

#ifndef MITHRIL_CRYPTO_H
#define MITHRIL_CRYPTO_H

/**
 * @file mithril_crypto.h
 * @brief Umbrella header forwarding to Mithril API V2.
 * 
 * Notice: New development should link directly against mithril_v2 and include
 * headers from <mithril/mithril_api.h>.
 */

#include <mithril/mithril_api.h>
#include <mithril/mithril_aead.h>
#include <mithril/mithril_sign.h>
#include <mithril/mithril_kex.h>
#include <mithril/mithril_hash.h>
#include <mithril/mithril_rng.h>
#include <mithril/mithril_bigint.h>
#include <mithril/mithril_modarith.h>
#include <mithril/mithril_version.h>

const char *mithril_crypto_layer_info(void);

#endif /* MITHRIL_CRYPTO_H */
