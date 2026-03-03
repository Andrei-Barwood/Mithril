#include "mithril/mithril_kex.h"

#include "../core/core_internal.h"

size_t mithril_kex_public_key_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->kex_public_key_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->kex_public_key_size(active->descriptor.user_data, algorithm);
}

size_t mithril_kex_secret_key_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->kex_secret_key_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->kex_secret_key_size(active->descriptor.user_data, algorithm);
}

size_t mithril_kex_shared_secret_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->kex_shared_secret_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->kex_shared_secret_size(active->descriptor.user_data, algorithm);
}

mithril_status mithril_kex_keypair(
    mithril_context *ctx,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || public_key == 0 || secret_key == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->kex_keypair == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->kex_keypair(
        active->descriptor.user_data,
        algorithm,
        public_key,
        public_key_len,
        secret_key,
        secret_key_len));
}

mithril_status mithril_kex_shared_secret(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *my_secret_key,
    size_t my_secret_key_len,
    const uint8_t *peer_public_key,
    size_t peer_public_key_len,
    uint8_t *shared_secret,
    size_t shared_secret_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || my_secret_key == 0 || peer_public_key == 0 || shared_secret == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->kex_shared_secret == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return mithril_core_normalize_status(active->descriptor.crypto_ops->kex_shared_secret(
        active->descriptor.user_data,
        algorithm,
        my_secret_key,
        my_secret_key_len,
        peer_public_key,
        peer_public_key_len,
        shared_secret,
        shared_secret_len,
        written_len));
}
