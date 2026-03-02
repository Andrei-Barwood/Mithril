#include "mithril/mithril_aead.h"

#include "../core/core_internal.h"

size_t mithril_aead_key_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->aead_key_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->aead_key_size(active->descriptor.user_data, algorithm);
}

size_t mithril_aead_nonce_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->aead_nonce_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->aead_nonce_size(active->descriptor.user_data, algorithm);
}

size_t mithril_aead_tag_size(mithril_context *ctx, uint32_t algorithm) {
    const mithril_provider_entry *active;

    if (ctx == 0) {
        return 0u;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->aead_tag_size == 0) {
        return 0u;
    }

    return active->descriptor.crypto_ops->aead_tag_size(active->descriptor.user_data, algorithm);
}

mithril_status mithril_aead_encrypt(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *key,
    size_t key_len,
    const uint8_t *nonce,
    size_t nonce_len,
    const uint8_t *aad,
    size_t aad_len,
    const uint8_t *plaintext,
    size_t plaintext_len,
    uint8_t *ciphertext,
    size_t ciphertext_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || key == 0 || nonce == 0 || plaintext == 0 || ciphertext == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->aead_encrypt == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->aead_encrypt(
        active->descriptor.user_data,
        algorithm,
        key,
        key_len,
        nonce,
        nonce_len,
        aad,
        aad_len,
        plaintext,
        plaintext_len,
        ciphertext,
        ciphertext_len,
        written_len);
}

mithril_status mithril_aead_decrypt(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *key,
    size_t key_len,
    const uint8_t *nonce,
    size_t nonce_len,
    const uint8_t *aad,
    size_t aad_len,
    const uint8_t *ciphertext,
    size_t ciphertext_len,
    uint8_t *plaintext,
    size_t plaintext_len,
    size_t *written_len) {
    const mithril_provider_entry *active;

    if (ctx == 0 || key == 0 || nonce == 0 || ciphertext == 0 || plaintext == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    active = mithril_core_get_active_provider(ctx);
    if (active == 0 || active->descriptor.crypto_ops == 0 || active->descriptor.crypto_ops->aead_decrypt == 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    return active->descriptor.crypto_ops->aead_decrypt(
        active->descriptor.user_data,
        algorithm,
        key,
        key_len,
        nonce,
        nonce_len,
        aad,
        aad_len,
        ciphertext,
        ciphertext_len,
        plaintext,
        plaintext_len,
        written_len);
}
