#ifndef MITHRIL_RNG_H
#define MITHRIL_RNG_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

mithril_status mithril_rng_fill(mithril_context *ctx, uint8_t *out, size_t out_len);
mithril_status mithril_rng_u64(mithril_context *ctx, uint64_t *out_value);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_RNG_H */
