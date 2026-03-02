#include "provider_sodium_internal.h"

#if defined(MITHRIL_USE_LIBSODIUM)
#include <sodium.h>
#endif

mithril_status mithril_provider_sodium_rng_fill(void *user_data, uint8_t *out, size_t out_len) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || out == 0 || out_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    randombytes_buf(out, out_len);
    return MITHRIL_OK;
#else
    (void)state;
    (void)out;
    (void)out_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_sodium_on_activate(void *user_data) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    if (sodium_init() < 0) {
        return MITHRIL_ERR_INTERNAL;
    }

    state->initialized = 1;
    return MITHRIL_OK;
#else
    (void)state;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

void mithril_provider_sodium_on_deactivate(void *user_data) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0) {
        return;
    }

    state->initialized = 0;
}

mithril_status mithril_provider_sodium_get_capabilities(void *user_data, mithril_capabilities *out_caps) {
    mithril_provider_sodium_state *state = (mithril_provider_sodium_state *)user_data;

    if (state == 0 || out_caps == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_LIBSODIUM)
    out_caps->feature_flags = MITHRIL_CAP_RNG | MITHRIL_CAP_HASH | MITHRIL_CAP_AEAD | MITHRIL_CAP_KEX | MITHRIL_CAP_SIGN;
    out_caps->provider_count = 1u;
    out_caps->reserved = 0u;
    return MITHRIL_OK;
#else
    (void)state;
    (void)out_caps;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
