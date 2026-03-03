#include "mithril/mithril_bigint.h"

#include "../core/core_internal.h"

mithril_status mithril_bigint_add(
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || a == 0 || b == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->bigint_add == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->bigint_add(
        active->descriptor.user_data,
        a,
        a_len,
        b,
        b_len,
        out,
        out_len,
        written_len));
}

mithril_status mithril_bigint_sub(
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || a == 0 || b == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->bigint_sub == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->bigint_sub(
        active->descriptor.user_data,
        a,
        a_len,
        b,
        b_len,
        out,
        out_len,
        written_len));
}

mithril_status mithril_bigint_mul(
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || a == 0 || b == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->bigint_mul == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->bigint_mul(
        active->descriptor.user_data,
        a,
        a_len,
        b,
        b_len,
        out,
        out_len,
        written_len));
}
