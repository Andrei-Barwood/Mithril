#include "provider_sodium_internal.h"

#include "mithril/mithril_kex.h"

#if defined(MITHRIL_USE_LIBSODIUM)
#include <sodium.h>
#endif

size_t mithril_provider_sodium_kex_public_key_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_KEX_ALG_X25519) {
        return 32u;
    }
    return 0u;
}

size_t mithril_provider_sodium_kex_secret_key_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_KEX_ALG_X25519) {
        return 32u;
    }
    return 0u;
}

size_t mithril_provider_sodium_kex_shared_secret_size(void *user_data, uint32_t algorithm) {
    (void)user_data;
    if (algorithm == MITHRIL_KEX_ALG_X25519) {
        return 32u;
    }
    return 0u;
}

mithril_status mithril_provider_sodium_kex_keypair(
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

    if (algorithm != MITHRIL_KEX_ALG_X25519 ||
        public_key_len != mithril_provider_sodium_kex_public_key_size(user_data, algorithm) ||
        secret_key_len != mithril_provider_sodium_kex_secret_key_size(user_data, algorithm)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    randombytes_buf(secret_key, secret_key_len);
    crypto_scalarmult_base(public_key, secret_key);
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_sodium_kex_shared_secret(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *my_secret_key,
    size_t my_secret_key_len,
    const uint8_t *peer_public_key,
    size_t peer_public_key_len,
    uint8_t *shared_secret,
    size_t shared_secret_len,
    size_t *written_len) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || my_secret_key == 0 || peer_public_key == 0 || shared_secret == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (algorithm != MITHRIL_KEX_ALG_X25519 ||
        my_secret_key_len != mithril_provider_sodium_kex_secret_key_size(user_data, algorithm) ||
        peer_public_key_len != mithril_provider_sodium_kex_public_key_size(user_data, algorithm) ||
        shared_secret_len < mithril_provider_sodium_kex_shared_secret_size(user_data, algorithm)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    if (crypto_scalarmult(shared_secret, my_secret_key, peer_public_key) != 0) {
        return MITHRIL_ERR_INTERNAL;
    }

    *written_len = mithril_provider_sodium_kex_shared_secret_size(user_data, algorithm);
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
