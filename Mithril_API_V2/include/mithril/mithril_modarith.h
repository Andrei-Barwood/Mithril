#ifndef MITHRIL_MODARITH_H
#define MITHRIL_MODARITH_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

mithril_status mithril_modarith_add_mod(
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

mithril_status mithril_modarith_mul_mod(
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_MODARITH_H */
