#include "mithril/mithril_sign.h"

#include "../core/core_internal.h"

size_t mithril_sign_public_key_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_public_key_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->sign_public_key_size(active->descriptor.user_data, algorithm);
}

size_t mithril_sign_secret_key_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_secret_key_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->sign_secret_key_size(active->descriptor.user_data, algorithm);
}

size_t mithril_sign_signature_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_signature_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->sign_signature_size(active->descriptor.user_data, algorithm);
}

mithril_status mithril_sign_keypair(
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
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_keypair == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->sign_keypair(
        active->descriptor.user_data,
        algorithm,
        public_key,
        public_key_len,
        secret_key,
        secret_key_len);
}

mithril_status mithril_sign_detached(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *secret_key,
    size_t secret_key_len,
    uint8_t *signature,
    size_t signature_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || message == 0 || secret_key == 0 || signature == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_detached == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->sign_detached(
        active->descriptor.user_data,
        algorithm,
        message,
        message_len,
        secret_key,
        secret_key_len,
        signature,
        signature_len,
        written_len);
}

mithril_status mithril_sign_verify_detached(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *public_key,
    size_t public_key_len,
    const uint8_t *signature,
    size_t signature_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || message == 0 || public_key == 0 || signature == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->sign_verify_detached == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->sign_verify_detached(
        active->descriptor.user_data,
        algorithm,
        message,
        message_len,
        public_key,
        public_key_len,
        signature,
        signature_len);
}
