#include "provider_sodium_internal.h"

#include "mithril/mithril_sign.h"

#if defined(MITHRIL_USE_LIBSODIUM)
#include <sodium.h>
#endif

size_t mithril_provider_sodium_sign_public_key_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_SIGN_ALG_ED25519) {
        return 32u;
    }
    return 0u;
}

size_t mithril_provider_sodium_sign_secret_key_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_SIGN_ALG_ED25519) {
        return 64u;
    }
    return 0u;
}

size_t mithril_provider_sodium_sign_signature_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_SIGN_ALG_ED25519) {
        return 64u;
    }
    return 0u;
}

mithril_status mithril_provider_sodium_sign_keypair(
    void *user_data,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || public_key == 0 || secret_key == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_SIGN_ALG_ED25519 ||
        public_key_len != mithril_provider_sodium_sign_public_key_size(user_data, algorithm) ||
        secret_key_len != mithril_provider_sodium_sign_secret_key_size(user_data, algorithm)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    if (crypto_sign_keypair(public_key, secret_key) != 0) {
        return MITHRIL_ERR_INTERNAL;
    }
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_sodium_sign_detached(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *secret_key,
    size_t secret_key_len,
    uint8_t *signature,
    size_t signature_len,
    size_t *written_len) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || message == 0 || secret_key == 0 || signature == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_SIGN_ALG_ED25519 ||
        secret_key_len != mithril_provider_sodium_sign_secret_key_size(user_data, algorithm) ||
        signature_len < mithril_provider_sodium_sign_signature_size(user_data, algorithm)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    {
        unsigned long long sig_len = 0ULL;
        if (crypto_sign_detached(signature, &sig_len, message, (unsigned long long)message_len, secret_key) != 0) {
            return MITHRIL_ERR_INTERNAL;
        }
        *written_len = (size_t)sig_len;
    }
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_sodium_sign_verify_detached(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *public_key,
    size_t public_key_len,
    const uint8_t *signature,
    size_t signature_len) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || message == 0 || public_key == 0 || signature == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_SIGN_ALG_ED25519 ||
        public_key_len != mithril_provider_sodium_sign_public_key_size(user_data, algorithm) ||
        signature_len != mithril_provider_sodium_sign_signature_size(user_data, algorithm)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    if (crypto_sign_verify_detached(signature, message, (unsigned long long)message_len, public_key) != 0) {
        return MITHRIL_ERR_INTERNAL;
    }
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
