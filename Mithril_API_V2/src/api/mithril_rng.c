#include "mithril/mithril_rng.h"

#include "../core/core_internal.h"

mithril_status mithril_rng_fill(mithril_context *ctx, uint8_t *out, size_t out_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || out == 0 || out_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->rng_fill == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->rng_fill(active->descriptor.user_data, out, out_len);
}

mithril_status mithril_rng_u64(mithril_context *ctx, uint64_t *out_value) {
    uint8_t bytes[8];
    size_t i;
    uint64_t value;
    mithril_status status;

    if (out_value == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    status = mithril_rng_fill(ctx, bytes, sizeof(bytes));
    if (status != MITHRIL_OK) {
        return status;
    }

    value = 0u;
    for (i = 0u; i < sizeof(bytes); ++i) {
        value = (value << 8u) | (uint64_t)bytes[i];
    }

    *out_value = value;
    return MITHRIL_OK;
}
