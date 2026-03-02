#include "mithril/mithril_hash.h"

#include "../core/core_internal.h"

size_t mithril_hash_digest_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->hash_digest_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->hash_digest_size(active->descriptor.user_data, algorithm);
}

mithril_status mithril_hash_compute(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *input,
    size_t input_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || input == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->hash_compute == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->hash_compute(
        active->descriptor.user_data,
        algorithm,
        input,
        input_len,
        out,
        out_len,
        written_len);
}
