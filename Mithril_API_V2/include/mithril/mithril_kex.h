#ifndef MITHRIL_KEX_H
#define MITHRIL_KEX_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

enum {
    MITHRIL_KEX_ALG_X25519 = 1u
};

#ifdef __cplusplus
extern "C" {
#endif

size_t mithril_kex_public_key_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_kex_secret_key_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_kex_shared_secret_size(mithril_context *ctx, uint32_t algorithm);

mithril_status mithril_kex_keypair(
    mithril_context *ctx,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len);

mithril_status mithril_kex_shared_secret(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *my_secret_key,
    size_t my_secret_key_len,
    const uint8_t *peer_public_key,
    size_t peer_public_key_len,
    uint8_t *shared_secret,
    size_t shared_secret_len,
    size_t *written_len);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_KEX_H */
