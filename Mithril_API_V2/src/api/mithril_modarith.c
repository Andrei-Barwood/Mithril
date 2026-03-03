#include "mithril/mithril_modarith.h"

#include "../core/core_internal.h"

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
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || a == 0 || b == 0 || modulus == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->modarith_add_mod == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->modarith_add_mod(
        active->descriptor.user_data,
        a,
        a_len,
        b,
        b_len,
        modulus,
        modulus_len,
        out,
        out_len,
        written_len));
}

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
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || a == 0 || b == 0 || modulus == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->modarith_mul_mod == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->modarith_mul_mod(
        active->descriptor.user_data,
        a,
        a_len,
        b,
        b_len,
        modulus,
        modulus_len,
        out,
        out_len,
        written_len));
}
