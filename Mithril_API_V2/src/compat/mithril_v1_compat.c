#include "mithril/compat/mithril_v1_compat.h"

#include <stdlib.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_error.h"
#include "mithril/mithril_modarith.h"
#include "mithril/mithril_provider.h"
#include "mithril_v1_compat_internal.h"

#if defined(MITHRIL_USE_FLINT)
#include <flint/fmpz.h>
#include "../providers/provider_flint/provider_flint_internal.h"
#endif

const uint8_t mithril_v1_ed25519_order_l_be[MITHRIL_V1_COMPAT_SCALAR_BYTES] = {
    0x10u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x14u, 0xDEu, 0xF9u, 0xDEu, 0xA2u, 0xF7u, 0x9Cu, 0xD6u,
    0x58u, 0x12u, 0x63u, 0x1Au, 0x5Cu, 0xF5u, 0xD3u, 0xEDu
};

static mithril_context *g_v1_ctx = NULL;

static int mithril_v1_activate_math_provider(mithril_context *ctx) {
    mithril_status st;

#if defined(MITHRIL_ENABLE_PROVIDER_FLINT) && defined(MITHRIL_USE_FLINT)
    st = mithril_provider_activate(ctx, "flint");
    if (st == MITHRIL_OK) {
        return 1;
    }
#endif

    st = mithril_provider_activate(ctx, "c23");
    if (st == MITHRIL_OK) {
        return 1;
    }

    st = mithril_provider_activate(ctx, "flint");
    if (st == MITHRIL_OK) {
        return 1;
    }

    return 0;
}

static int mithril_v1_ensure_math_context(mithril_context **out_ctx) {
    if (out_ctx == NULL) {
        return 0;
    }

    if (mithril_v1_compat_init() != 0) {
        return 0;
    }

    *out_ctx = g_v1_ctx;
    return 1;
}

static void mithril_v1_reverse_copy(uint8_t *out, const uint8_t *in, size_t len) {
    size_t i;

    for (i = 0u; i < len; ++i) {
        out[i] = in[len - 1u - i];
    }
}

static int mithril_v1_be_to_fixed_le(
    const uint8_t *be,
    size_t be_len,
    uint8_t *out_le,
    size_t fixed_len,
    int *overflow) {
    size_t copy_len;
    size_t offset;
    size_t i;

    if (be == NULL || out_le == NULL || fixed_len == 0u) {
        return 0;
    }

    memset(out_le, 0, fixed_len);
    copy_len = (be_len > fixed_len) ? fixed_len : be_len;
    offset = be_len - copy_len;

    if (overflow != NULL) {
        *overflow = 0;
        for (i = 0u; i < offset; ++i) {
            if (be[i] != 0u) {
                *overflow = 1;
                break;
            }
        }
    }

    for (i = 0u; i < copy_len; ++i) {
        out_le[i] = be[be_len - 1u - i];
    }

    return 1;
}

static int mithril_v1_cmp_le(const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;

    for (i = len; i > 0u; --i) {
        uint8_t av = a[i - 1u];
        uint8_t bv = b[i - 1u];
        if (av < bv) {
            return -1;
        }
        if (av > bv) {
            return 1;
        }
    }

    return 0;
}

static void mithril_v1_sub_one_le(uint8_t *value, size_t len) {
    size_t i;

    for (i = 0u; i < len; ++i) {
        if (value[i] > 0u) {
            value[i] = (uint8_t)(value[i] - 1u);
            return;
        }
        value[i] = 0xFFu;
    }
}

static int mithril_v1_bigint_sub_fixed_le(
    mithril_context *ctx,
    const uint8_t *x_le,
    const uint8_t *y_le,
    size_t len,
    uint8_t *out_le) {
    uint8_t *x_be;
    uint8_t *y_be;
    uint8_t *raw_be;
    size_t raw_len = 0u;
    mithril_status st;
    int ok = 0;

    x_be = (uint8_t *)calloc(len, 1u);
    y_be = (uint8_t *)calloc(len, 1u);
    raw_be = (uint8_t *)calloc(len + 1u, 1u);
    if (x_be == NULL || y_be == NULL || raw_be == NULL) {
        goto cleanup;
    }

    mithril_v1_reverse_copy(x_be, x_le, len);
    mithril_v1_reverse_copy(y_be, y_le, len);

    st = mithril_bigint_sub(ctx, x_be, len, y_be, len, raw_be, len + 1u, &raw_len);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    if (!mithril_v1_be_to_fixed_le(raw_be, raw_len, out_le, len, NULL)) {
        goto cleanup;
    }

    ok = 1;

cleanup:
    free(x_be);
    free(y_be);
    free(raw_be);
    return ok;
}

#if defined(MITHRIL_USE_FLINT)
static int mithril_v1_fmpz_to_be_alloc(const fmpz_t value, uint8_t **out, size_t *out_len) {
    size_t bits;
    size_t capacity;
    mithril_status st;

    if (out == NULL || out_len == NULL) {
        return 0;
    }

    if (fmpz_sgn(value) < 0) {
        return 0;
    }

    bits = (size_t)fmpz_bits(value);
    capacity = (bits == 0u) ? 1u : ((bits + 7u) / 8u);

    *out = (uint8_t *)calloc(capacity, 1u);
    if (*out == NULL) {
        return 0;
    }

    st = mithril_flint_fmpz_to_be(value, *out, capacity, out_len);
    if (st != MITHRIL_OK) {
        free(*out);
        *out = NULL;
        *out_len = 0u;
        return 0;
    }

    return 1;
}

static int mithril_v1_ulong_to_be_alloc(ulong value, uint8_t **out, size_t *out_len) {
    ulong tmp = value;
    size_t len = 0u;
    size_t i;

    if (out == NULL || out_len == NULL) {
        return 0;
    }

    if (tmp == 0u) {
        len = 1u;
    } else {
        while (tmp > 0u) {
            ++len;
            tmp >>= 8u;
        }
    }

    *out = (uint8_t *)calloc(len, 1u);
    if (*out == NULL) {
        return 0;
    }

    for (i = 0u; i < len; ++i) {
        (*out)[len - 1u - i] = (uint8_t)(value & 0xFFu);
        value >>= 8u;
    }

    *out_len = len;
    return 1;
}

static void mithril_v1_fmpz_from_limbs(
    fmpz_t out,
    const mp_limb_t *limbs,
    slong limb_count) {
    slong i;

    fmpz_zero(out);
    if (limbs == NULL || limb_count <= 0) {
        return;
    }

    for (i = limb_count - 1; i >= 0; --i) {
        fmpz_mul_2exp(out, out, (ulong)FLINT_BITS);
        if (limbs[i] != (mp_limb_t)0) {
            fmpz_add_ui(out, out, (ulong)limbs[i]);
        }
    }
}
#endif

int mithril_v1_compat_init(void) {
    mithril_status st;

    if (g_v1_ctx != NULL) {
        return mithril_v1_activate_math_provider(g_v1_ctx) ? 0 : -1;
    }

    st = mithril_init(&g_v1_ctx, NULL);
    if (st != MITHRIL_OK) {
        g_v1_ctx = NULL;
        return -1;
    }

    if (!mithril_v1_activate_math_provider(g_v1_ctx)) {
        mithril_shutdown(g_v1_ctx);
        g_v1_ctx = NULL;
        return -1;
    }

    return 0;
}

void mithril_v1_compat_shutdown(void) {
    if (g_v1_ctx != NULL) {
        mithril_shutdown(g_v1_ctx);
        g_v1_ctx = NULL;
    }
}

int mithril_add_scalar(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]) {
    mithril_context *ctx;
    uint8_t a_be[MITHRIL_SCALAR_BYTES];
    uint8_t b_be[MITHRIL_SCALAR_BYTES];
    uint8_t sum_be[MITHRIL_SCALAR_BYTES + 1u];
    size_t sum_len = 0u;
    int overflow = 0;
    mithril_status st;

    if (result == NULL || a == NULL || b == NULL) {
        return -1;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    mithril_v1_reverse_copy(a_be, a, MITHRIL_SCALAR_BYTES);
    mithril_v1_reverse_copy(b_be, b, MITHRIL_SCALAR_BYTES);

    st = mithril_bigint_add(
        ctx,
        a_be,
        MITHRIL_SCALAR_BYTES,
        b_be,
        MITHRIL_SCALAR_BYTES,
        sum_be,
        sizeof(sum_be),
        &sum_len);
    if (st != MITHRIL_OK) {
        return -1;
    }

    if (!mithril_v1_be_to_fixed_le(sum_be, sum_len, result, MITHRIL_SCALAR_BYTES, &overflow)) {
        return -1;
    }

    return overflow ? 1 : 0;
}

int mithril_add_mod_l(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]) {
    mithril_context *ctx;
    uint8_t a_be[MITHRIL_SCALAR_BYTES];
    uint8_t b_be[MITHRIL_SCALAR_BYTES];
    uint8_t out_be[MITHRIL_SCALAR_BYTES];
    size_t out_len = 0u;
    mithril_status st;

    if (result == NULL || a == NULL || b == NULL) {
        return -1;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    mithril_v1_reverse_copy(a_be, a, MITHRIL_SCALAR_BYTES);
    mithril_v1_reverse_copy(b_be, b, MITHRIL_SCALAR_BYTES);

    st = mithril_modarith_add_mod(
        ctx,
        a_be,
        MITHRIL_SCALAR_BYTES,
        b_be,
        MITHRIL_SCALAR_BYTES,
        mithril_v1_ed25519_order_l_be,
        MITHRIL_SCALAR_BYTES,
        out_be,
        sizeof(out_be),
        &out_len);
    if (st != MITHRIL_OK) {
        return -1;
    }

    if (!mithril_v1_be_to_fixed_le(out_be, out_len, result, MITHRIL_SCALAR_BYTES, NULL)) {
        return -1;
    }

    return 0;
}

int mithril_add_constant_time(uint8_t *result, const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;
    uint16_t carry = 0u;

    if (result == NULL || a == NULL || b == NULL || len == 0u) {
        return -1;
    }

    for (i = 0u; i < len; ++i) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        result[i] = (uint8_t)(sum & 0xFFu);
        carry = (uint16_t)(sum >> 8u);
    }

    return (carry != 0u) ? 1 : 0;
}

int sodium_sub_with_underflow(unsigned char *result, const unsigned char *a, const unsigned char *b, size_t len) {
    mithril_context *ctx;
    int cmp;

    if (result == NULL || a == NULL || b == NULL || len == 0u) {
        return -1;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    cmp = mithril_v1_cmp_le(a, b, len);
    if (cmp >= 0) {
        if (!mithril_v1_bigint_sub_fixed_le(ctx, a, b, len, result)) {
            return -1;
        }
        return E_SODIUM_OK;
    }

    if (!mithril_v1_bigint_sub_fixed_le(ctx, b, a, len, result)) {
        return -1;
    }

    mithril_v1_sub_one_le(result, len);
    return E_SODIUM_UFL;
}

#if defined(MITHRIL_USE_FLINT)
int fmpz_mul_safe(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *b_be = NULL;
    uint8_t *out_be = NULL;
    size_t a_len = 0u;
    size_t b_len = 0u;
    size_t out_len = 0u;
    size_t out_cap;
    mithril_status st;
    int rc = -1;

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup;
    }
    if (!mithril_v1_fmpz_to_be_alloc(b, &b_be, &b_len)) {
        goto cleanup;
    }

    out_cap = a_len + b_len + 1u;
    out_be = (uint8_t *)calloc(out_cap, 1u);
    if (out_be == NULL) {
        goto cleanup;
    }

    st = mithril_bigint_mul(ctx, a_be, a_len, b_be, b_len, out_be, out_cap, &out_len);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    st = mithril_flint_fmpz_from_be(out_be, out_len, result);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    rc = 0;

cleanup:
    free(a_be);
    free(b_be);
    free(out_be);
    return rc;
}

int fmpz_square_safe(fmpz_t result, const fmpz_t input) {
    return fmpz_mul_safe(result, input, input);
}

int fmpz_inc(fmpz_t result, const fmpz_t a) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *out_be = NULL;
    size_t a_len = 0u;
    size_t out_len = 0u;
    const uint8_t one_be[] = {0x01u};
    mithril_status st;
    int rc = -1;

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup;
    }

    out_be = (uint8_t *)calloc(a_len + 1u, 1u);
    if (out_be == NULL) {
        goto cleanup;
    }

    st = mithril_bigint_add(ctx, a_be, a_len, one_be, sizeof(one_be), out_be, a_len + 1u, &out_len);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    st = mithril_flint_fmpz_from_be(out_be, out_len, result);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    rc = E_FMPZ_OK;

cleanup:
    free(a_be);
    free(out_be);
    return rc;
}

int fmpz_inc_inplace(fmpz_t a) {
    return fmpz_inc(a, a);
}

int fmpz_inc_mod(fmpz_t result, const fmpz_t a, const fmpz_t mod) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *mod_be = NULL;
    uint8_t *out_be = NULL;
    size_t a_len = 0u;
    size_t mod_len = 0u;
    size_t out_len = 0u;
    size_t out_cap;
    const uint8_t one_be[] = {0x01u};
    mithril_status st;
    int rc = -1;

    if (fmpz_sgn(mod) <= 0) {
        return -1;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup;
    }
    if (!mithril_v1_fmpz_to_be_alloc(mod, &mod_be, &mod_len)) {
        goto cleanup;
    }

    out_cap = ((a_len > mod_len) ? a_len : mod_len) + 1u;
    out_be = (uint8_t *)calloc(out_cap, 1u);
    if (out_be == NULL) {
        goto cleanup;
    }

    st = mithril_modarith_add_mod(
        ctx,
        a_be,
        a_len,
        one_be,
        sizeof(one_be),
        mod_be,
        mod_len,
        out_be,
        out_cap,
        &out_len);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    st = mithril_flint_fmpz_from_be(out_be, out_len, result);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    rc = E_FMPZ_OK;

cleanup:
    free(a_be);
    free(mod_be);
    free(out_be);
    return rc;
}

int fmpz_dec(fmpz_t a) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *out_be = NULL;
    size_t a_len = 0u;
    size_t out_len = 0u;
    const uint8_t one_be[] = {0x01u};
    mithril_status st;
    int rc = E_FMPZ_UFL;

    if (fmpz_is_zero(a)) {
        fmpz_set_si(a, -1);
        return E_FMPZ_UFL;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return E_FMPZ_UFL;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup;
    }

    out_be = (uint8_t *)calloc(a_len, 1u);
    if (out_be == NULL) {
        goto cleanup;
    }

    st = mithril_bigint_sub(ctx, a_be, a_len, one_be, sizeof(one_be), out_be, a_len, &out_len);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    st = mithril_flint_fmpz_from_be(out_be, out_len, a);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    rc = E_FMPZ_OK;

cleanup:
    free(a_be);
    free(out_be);
    return rc;
}

int fmpz_sub_ushort(fmpz_t result, const fmpz_t a, unsigned short b) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *b_be = NULL;
    uint8_t *out_be = NULL;
    size_t a_len = 0u;
    size_t b_len = 0u;
    size_t out_len = 0u;
    size_t out_cap;
    fmpz_t b_f;
    mithril_status st;
    int rc = -1;
    int cmp;

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup_preinit;
    }
    if (!mithril_v1_ulong_to_be_alloc((ulong)b, &b_be, &b_len)) {
        goto cleanup_preinit;
    }

    out_cap = ((a_len > b_len) ? a_len : b_len);
    if (out_cap == 0u) {
        out_cap = 1u;
    }
    out_be = (uint8_t *)calloc(out_cap, 1u);
    if (out_be == NULL) {
        goto cleanup_preinit;
    }

    fmpz_init(b_f);
    fmpz_set_ui(b_f, (ulong)b);
    cmp = fmpz_cmp(a, b_f);

    if (cmp >= 0) {
        st = mithril_bigint_sub(ctx, a_be, a_len, b_be, b_len, out_be, out_cap, &out_len);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }
        st = mithril_flint_fmpz_from_be(out_be, out_len, result);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }
    } else {
        st = mithril_bigint_sub(ctx, b_be, b_len, a_be, a_len, out_be, out_cap, &out_len);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }
        st = mithril_flint_fmpz_from_be(out_be, out_len, result);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }
        fmpz_neg(result, result);
    }

    rc = E_FMPZ_OK;

cleanup:
    fmpz_clear(b_f);
cleanup_preinit:
    free(a_be);
    free(b_be);
    free(out_be);
    return rc;
}

int fmpz_mul_ui_mod(fmpz_t result, const fmpz_t a, ulong b, const fmpz_t modulus) {
    mithril_context *ctx;
    uint8_t *a_be = NULL;
    uint8_t *b_be = NULL;
    uint8_t *mod_be = NULL;
    uint8_t *mul_be = NULL;
    uint8_t *red_be = NULL;
    size_t a_len = 0u;
    size_t b_len = 0u;
    size_t mod_len = 0u;
    size_t mul_len = 0u;
    size_t red_len = 0u;
    size_t mul_cap;
    size_t red_cap;
    mithril_status st;
    fmpz_t mul_f;
    int rc = -1;
    int overflow_flag = E_FLINT_OK;

    if (fmpz_sgn(modulus) <= 0) {
        return -1;
    }

    if (!mithril_v1_ensure_math_context(&ctx)) {
        return -1;
    }

    if (!mithril_v1_fmpz_to_be_alloc(a, &a_be, &a_len)) {
        goto cleanup_preinit;
    }
    if (!mithril_v1_ulong_to_be_alloc(b, &b_be, &b_len)) {
        goto cleanup_preinit;
    }
    if (!mithril_v1_fmpz_to_be_alloc(modulus, &mod_be, &mod_len)) {
        goto cleanup_preinit;
    }

    mul_cap = a_len + b_len + 1u;
    mul_be = (uint8_t *)calloc(mul_cap, 1u);
    if (mul_be == NULL) {
        goto cleanup_preinit;
    }

    st = mithril_bigint_mul(ctx, a_be, a_len, b_be, b_len, mul_be, mul_cap, &mul_len);
    if (st != MITHRIL_OK) {
        goto cleanup_preinit;
    }

    fmpz_init(mul_f);
    st = mithril_flint_fmpz_from_be(mul_be, mul_len, mul_f);
    if (st != MITHRIL_OK) {
        goto cleanup;
    }

    if (fmpz_cmp(mul_f, modulus) >= 0) {
        overflow_flag = E_FLINT_OFL;
        red_cap = mod_len + 1u;
        red_be = (uint8_t *)calloc(red_cap, 1u);
        if (red_be == NULL) {
            goto cleanup;
        }

        st = mithril_modarith_mul_mod(
            ctx,
            a_be,
            a_len,
            b_be,
            b_len,
            mod_be,
            mod_len,
            red_be,
            red_cap,
            &red_len);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }

        st = mithril_flint_fmpz_from_be(red_be, red_len, result);
        if (st != MITHRIL_OK) {
            goto cleanup;
        }
    } else {
        fmpz_set(result, mul_f);
    }

    rc = overflow_flag;

cleanup:
    fmpz_clear(mul_f);
cleanup_preinit:
    free(a_be);
    free(b_be);
    free(mod_be);
    free(mul_be);
    free(red_be);
    return rc;
}

int div_fmpz(fmpz_t quot, fmpz_t rem, const fmpz_t dividend, const fmpz_t divisor) {
    if (fmpz_is_zero(divisor)) {
        return -1;
    }

    if (fmpz_is_zero(dividend)) {
        fmpz_zero(quot);
        fmpz_zero(rem);
        return 0;
    }

    fmpz_tdiv_qr(quot, rem, dividend, divisor);
    return 0;
}

void rem_mod_pow_of_2(const fmpz_t x, ulong k, fmpz_t res) {
    fmpz_t mod;

    fmpz_init(mod);
    fmpz_set_ui(mod, 1uL);
    fmpz_mul_2exp(mod, mod, k);
    fmpz_mod(res, x, mod);
    fmpz_clear(mod);
}

void flint_kmul(const fmpz_t a, const fmpz_t b, fmpz_t result) {
    fmpz_mul(result, a, b);
}

void flint_kmul_limbs(
    const mp_limb_t *a_limbs,
    slong a_size,
    const mp_limb_t *b_limbs,
    slong b_size,
    fmpz_t result) {
    fmpz_t a;
    fmpz_t b;

    fmpz_init(a);
    fmpz_init(b);

    mithril_v1_fmpz_from_limbs(a, a_limbs, a_size);
    mithril_v1_fmpz_from_limbs(b, b_limbs, b_size);
    fmpz_mul(result, a, b);

    fmpz_clear(a);
    fmpz_clear(b);
}
#endif
