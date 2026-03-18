#include "provider_sodium_internal.h"

#include "mithril/mithril_aead.h"

#if defined(MITHRIL_USE_LIBSODIUM)
#include <sodium.h>
#endif

size_t mithril_provider_sodium_aead_key_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF) {
        return 32u;
    }
    return 0u;
}

size_t mithril_provider_sodium_aead_nonce_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF) {
        return 24u;
    }
    return 0u;
}

size_t mithril_provider_sodium_aead_tag_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF) {
        return 16u;
    }
    return 0u;
}

mithril_status mithril_provider_sodium_aead_encrypt(
    void *user_data,
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
    size_t need_len;
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || key == 0 || nonce == 0 || plaintext == 0 || ciphertext == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF) {
        return MITHRIL_ERR_NOT_IMPLEMENTED;
    }

    need_len = plaintext_len + mithril_provider_sodium_aead_tag_size(user_data, algorithm);
    if (key_len != mithril_provider_sodium_aead_key_size(user_data, algorithm) ||
        nonce_len != mithril_provider_sodium_aead_nonce_size(user_data, algorithm) ||
        ciphertext_len < need_len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    {
        unsigned long long out_len = 0ULL;
        if (crypto_aead_xchacha20poly1305_ietf_encrypt(
                ciphertext,
                &out_len,
                plaintext,
                (unsigned long long)plaintext_len,
                aad,
                (unsigned long long)aad_len,
                0,
                nonce,
                key) != 0) {
            return MITHRIL_ERR_INTERNAL;
        }
        *written_len = (size_t)out_len;
    }
    return MITHRIL_OK;
#else
    (void)aad;
    (void)aad_len;
    (void)need_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_sodium_aead_decrypt(
    void *user_data,
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
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || key == 0 || nonce == 0 || ciphertext == 0 || plaintext == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF) {
        return MITHRIL_ERR_NOT_IMPLEMENTED;
    }

    if (key_len != mithril_provider_sodium_aead_key_size(user_data, algorithm) ||
        nonce_len != mithril_provider_sodium_aead_nonce_size(user_data, algorithm) ||
        ciphertext_len < mithril_provider_sodium_aead_tag_size(user_data, algorithm) ||
        plaintext_len < (ciphertext_len - mithril_provider_sodium_aead_tag_size(user_data, algorithm))) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    {
        unsigned long long out_len = 0ULL;
        if (crypto_aead_xchacha20poly1305_ietf_decrypt(
                plaintext,
                &out_len,
                0,
                ciphertext,
                (unsigned long long)ciphertext_len,
                aad,
                (unsigned long long)aad_len,
                nonce,
                key) != 0) {
            return MITHRIL_ERR_AEAD_AUTH_FAILED;
        }
        *written_len = (size_t)out_len;
    }
    return MITHRIL_OK;
#else
    (void)aad;
    (void)aad_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
