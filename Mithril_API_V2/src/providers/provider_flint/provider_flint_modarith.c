#include "provider_flint_internal.h"

#if defined(MITHRIL_USE_FLINT)
#include <flint/fmpz.h>
#endif

mithril_status mithril_provider_flint_modarith_add_mod(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;

    if (state == 0 || a == NULL || b == NULL || modulus == NULL || out == NULL || written_len == NULL ||
        a_len == 0u || b_len == 0u || modulus_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_FLINT)
    {
        mithril_status st;
        fmpz_t fa;
        fmpz_t fb;
        fmpz_t fm;
        fmpz_t ft;
        fmpz_t fr;

        fmpz_init(fa);
        fmpz_init(fb);
        fmpz_init(fm);
        fmpz_init(ft);
        fmpz_init(fr);

        st = mithril_flint_fmpz_from_be(a, a_len, fa);
        if (st != MITHRIL_OK) { goto cleanup; }
        st = mithril_flint_fmpz_from_be(b, b_len, fb);
        if (st != MITHRIL_OK) { goto cleanup; }
        st = mithril_flint_fmpz_from_be(modulus, modulus_len, fm);
        if (st != MITHRIL_OK) { goto cleanup; }

        if (fmpz_is_zero(fm)) {
            st = MITHRIL_ERR_INVALID_ARGUMENT;
            goto cleanup;
        }

        fmpz_add(ft, fa, fb);
        fmpz_mod(fr, ft, fm);

        st = mithril_flint_fmpz_to_be(fr, out, out_len, written_len);

cleanup:
        fmpz_clear(fa);
        fmpz_clear(fb);
        fmpz_clear(fm);
        fmpz_clear(ft);
        fmpz_clear(fr);
        return st;
    }
#else
    (void)state; (void)a; (void)a_len; (void)b; (void)b_len; (void)modulus; (void)modulus_len; (void)out; (void)out_len; (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}

mithril_status mithril_provider_flint_modarith_mul_mod(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_flint_state *state = (mithril_provider_flint_state *)user_data;

    if (state == 0 || a == NULL || b == NULL || modulus == NULL || out == NULL || written_len == NULL ||
        a_len == 0u || b_len == 0u || modulus_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(MITHRIL_USE_FLINT)
    {
        mithril_status st;
        fmpz_t fa;
        fmpz_t fb;
        fmpz_t fm;
        fmpz_t ft;
        fmpz_t fr;

        fmpz_init(fa);
        fmpz_init(fb);
        fmpz_init(fm);
        fmpz_init(ft);
        fmpz_init(fr);

        st = mithril_flint_fmpz_from_be(a, a_len, fa);
        if (st != MITHRIL_OK) { goto cleanup; }
        st = mithril_flint_fmpz_from_be(b, b_len, fb);
        if (st != MITHRIL_OK) { goto cleanup; }
        st = mithril_flint_fmpz_from_be(modulus, modulus_len, fm);
        if (st != MITHRIL_OK) { goto cleanup; }

        if (fmpz_is_zero(fm)) {
            st = MITHRIL_ERR_INVALID_ARGUMENT;
            goto cleanup;
        }

        fmpz_mul(ft, fa, fb);
        fmpz_mod(fr, ft, fm);

        st = mithril_flint_fmpz_to_be(fr, out, out_len, written_len);

cleanup:
        fmpz_clear(fa);
        fmpz_clear(fb);
        fmpz_clear(fm);
        fmpz_clear(ft);
        fmpz_clear(fr);
        return st;
    }
#else
    (void)state; (void)a; (void)a_len; (void)b; (void)b_len; (void)modulus; (void)modulus_len; (void)out; (void)out_len; (void)written_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
