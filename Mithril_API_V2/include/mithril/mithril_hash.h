#ifndef MITHRIL_HASH_H
#define MITHRIL_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

enum {
    MITHRIL_HASH_ALG_BLAKE2B_256 = 1u
};

#ifdef __cplusplus
extern "C" {
#endif

size_t mithril_hash_digest_size(mithril_context *ctx, uint32_t algorithm);
mithril_status mithril_hash_compute(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *input,
    size_t input_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_HASH_H */
