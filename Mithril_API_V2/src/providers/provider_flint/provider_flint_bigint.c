#include "provider_flint_internal.h"

#include <stdlib.h>
#include <string.h>

#if defined(MITHRIL_USE_FLINT)
#include <flint/flint.h>
#include <flint/fmpz.h>

static int hex_nibble(char c, uint8_t *out) {
    if (c >= '0' && c <= '9') {
        *out = (uint8_t)(c - '0');
        return 1;
    }
    if (c >= 'a' && c <= 'f') {
        *out = (uint8_t)(10 + (c - 'a'));
        return 1;
    }
    if (c >= 'A' && c <= 'F') {
        *out = (uint8_t)(10 + (c - 'A'));
        return 1;
    }
    return 0;
}

mithril_status mithril_flint_fmpz_from_be(const uint8_t *in, size_t in_len, fmpz_t out) {
    size_t i;

    if (in == NULL || in_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    fmpz_zero(out);
    for (i = 0u; i < in_len; ++i) {
        fmpz_mul_2exp(out, out, 8u);
        if (in[i] != 0u) {
            fmpz_add_ui(out, out, (ulong)in[i]);
        }
    }

    return MITHRIL_OK;
}

mithril_status mithril_flint_fmpz_to_be(const fmpz_t in, uint8_t *out, size_t out_len, size_t *written_len) {
    char *hex;
    size_t hex_len;
    size_t byte_len;
    size_t i;
    size_t pos;

    if (out == NULL || written_len == NULL) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (fmpz_sgn(in) < 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (fmpz_is_zero(in)) {
        if (out_len < 1u) {
            return MITHRIL_ERR_INVALID_ARGUMENT;
        }
        out[0] = 0u;
        *written_len = 1u;
        return MITHRIL_OK;
    }

    hex = fmpz_get_str(NULL, 16, in);
    if (hex == NULL) {
        return MITHRIL_ERR_INTERNAL;
    }

    hex_len = strlen(hex);
    byte_len = (hex_len + 1u) / 2u;
    if (out_len < byte_len) {
        flint_free(hex);
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    pos = 0u;
    if ((hex_len % 2u) == 1u) {
        uint8_t hi;
        if (!hex_nibble(hex[0], &hi)) {
            flint_free(hex);
            return MITHRIL_ERR_INTERNAL;
        }
        out[pos++] = hi;
        i = 1u;
    } else {
        i = 0u;
    }

    while (i < hex_len) {
        uint8_t hi;
        uint8_t lo;
        if (!hex_nibble(hex[i], &hi) || !hex_nibble(hex[i + 1u], &lo)) {
            flint_free(hex);
            return MITHRIL_ERR_INTERNAL;
        }
        out[pos++] = (uint8_t)((hi << 4u) | lo);
        i += 2u;
    }

    *written_len = byte_len;
    flint_free(hex);
    return MITHRIL_OK;
}
#endif

mithril_status mithril_provider_flint_on_activate(void *user_data) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;
    if (state == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
#if defined(MITHRIL_USE_FLINT)
    state->initialized = 1;
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

void mithril_provider_flint_on_deactivate(void *user_data) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;
    if (state != 0) {
        state->initialized = 0;
    }
}

mithril_status mithril_provider_flint_get_capabilities(void *user_data, mithril_capabilities *out_caps) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;
    if (state == 0 || out_caps == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
#if defined(MITHRIL_USE_FLINT)
    out_caps->feature_flags = MITHRIL_CAP_BIGINT | MITHRIL_CAP_MODARITH;
    out_caps->provider_count = 1u;
    out_caps->reserved = 0u;
    return MITHRIL_OK;
#else
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_flint_bigint_add(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;

    if (state == 0 || a == NULL || b == NULL || out == NULL || written_len == NULL || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_FLINT)
    {
        mithril_status st;
        fmpz_t fa;
        fmpz_t fb;
        fmpz_t fr;

        fmpz_init(fa);
        fmpz_init(fb);
        fmpz_init(fr);

        st = mithril_flint_fmpz_from_be(a, a_len, fa);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        st = mithril_flint_fmpz_from_be(b, b_len, fb);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        fmpz_add(fr, fa, fb);
        st = mithril_flint_fmpz_to_be(fr, out, out_len, written_len);

        fmpz_clear(fa);
        fmpz_clear(fb);
        fmpz_clear(fr);
        return st;
    }
#else
    (void)state; (void)a; (void)a_len; (void)b; (void)b_len; (void)out; (void)out_len; (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_flint_bigint_sub(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;

    if (state == 0 || a == NULL || b == NULL || out == NULL || written_len == NULL || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_FLINT)
    {
        mithril_status st;
        fmpz_t fa;
        fmpz_t fb;
        fmpz_t fr;

        fmpz_init(fa);
        fmpz_init(fb);
        fmpz_init(fr);

        st = mithril_flint_fmpz_from_be(a, a_len, fa);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        st = mithril_flint_fmpz_from_be(b, b_len, fb);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        if (fmpz_cmp(fa, fb) < 0) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return MITHRIL_ERR_INVALID_ARGUMENT;
        }

        fmpz_sub(fr, fa, fb);
        st = mithril_flint_fmpz_to_be(fr, out, out_len, written_len);

        fmpz_clear(fa);
        fmpz_clear(fb);
        fmpz_clear(fr);
        return st;
    }
#else
    (void)state; (void)a; (void)a_len; (void)b; (void)b_len; (void)out; (void)out_len; (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_flint_bigint_mul(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;

    if (state == 0 || a == NULL || b == NULL || out == NULL || written_len == NULL || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_FLINT)
    {
        mithril_status st;
        fmpz_t fa;
        fmpz_t fb;
        fmpz_t fr;

        fmpz_init(fa);
        fmpz_init(fb);
        fmpz_init(fr);

        st = mithril_flint_fmpz_from_be(a, a_len, fa);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        st = mithril_flint_fmpz_from_be(b, b_len, fb);
        if (st != MITHRIL_OK) {
            fmpz_clear(fa); fmpz_clear(fb); fmpz_clear(fr);
            return st;
        }

        fmpz_mul(fr, fa, fb);
        st = mithril_flint_fmpz_to_be(fr, out, out_len, written_len);

        fmpz_clear(fa);
        fmpz_clear(fb);
        fmpz_clear(fr);
        return st;
    }
#else
    (void)state; (void)a; (void)a_len; (void)b; (void)b_len; (void)out; (void)out_len; (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
