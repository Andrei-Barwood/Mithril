#include "provider_sodium_internal.h"

#include "mithril/mithril_hash.h"

#if defined(MITHRIL_USE_LIBSODIUM)
#include <sodium.h>
#endif

size_t mithril_provider_sodium_hash_digest_size(void *user_data, uint32_t algorithm) {
    (void)user_data;

    if (algorithm == MITHRIL_HASH_ALG_BLAKE2B_256) {
        return 32u;
    }

    return 0u;
}

mithril_status mithril_provider_sodium_hash_compute(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *input,
    size_t input_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    size_t digest_size;
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || input == 0 || out == 0 || written_len == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    digest_size = mithril_provider_sodium_hash_digest_size(user_data, algorithm);
    if (digest_size == 0u || out_len < digest_size) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    if (algorithm != MITHRIL_HASH_ALG_BLAKE2B_256) {
        return MITHRIL_ERR_NOT_IMPLEMENTED;
    }

    if (crypto_generichash(out, digest_size, input, (unsigned long long)input_len, 0, 0) != 0) {
        return MITHRIL_ERR_INTERNAL;
    }

    *written_len = digest_size;
    return MITHRIL_OK;
#else
    (void)state;
    (void)algorithm;
    (void)input;
    (void)input_len;
    (void)out;
    (void)out_len;
    (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
