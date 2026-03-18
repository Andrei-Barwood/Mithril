#include "provider_c23_internal.h"

#include <stdlib.h>

static const uint8_t *skip_leading_zeros_be(const uint8_t *in, size_t *len) {
    size_t i = 0u;

    while (i + 1u < *len && in[i] == 0u) {
        ++i;
    }

    *len -= i;
    return in + i;
}

static int ensure_capacity(uint8_t **buf, size_t *cap, size_t need) {
    uint8_t *next;
    size_t i;

    if (*cap >= need) {
        return 1;
    }

    next = (uint8_t *)realloc(*buf, need);
    if (next == NULL) {
        return 0;
    }

    for (i = *cap; i < need; ++i) {
        next[i] = 0u;
    }

    *buf = next;
    *cap = need;
    return 1;
}

static void normalize_le(uint8_t *v, size_t *len) {
    while (*len > 1u && v[*len - 1u] == 0u) {
        --(*len);
    }
}

static int be_to_le_alloc(const uint8_t *be, size_t be_len, uint8_t **out, size_t *out_len) {
    size_t i;

    *out = (uint8_t *)calloc(be_len, 1u);
    if (*out == NULL) {
        return 0;
    }

    for (i = 0u; i < be_len; ++i) {
        (*out)[i] = be[be_len - 1u - i];
    }

    *out_len = be_len;
    normalize_le(*out, out_len);
    return 1;
}

static int cmp_le(const uint8_t *a, size_t a_len, const uint8_t *b, size_t b_len) {
    size_t i;

    while (a_len > 1u && a[a_len - 1u] == 0u) {
        --a_len;
    }
    while (b_len > 1u && b[b_len - 1u] == 0u) {
        --b_len;
    }

    if (a_len < b_len) {
        return -1;
    }
    if (a_len > b_len) {
        return 1;
    }

    for (i = a_len; i > 0u; --i) {
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

static void sub_le_inplace(uint8_t *a, size_t *a_len, const uint8_t *b, size_t b_len) {
    size_t i;
    int borrow = 0;

    for (i = 0u; i < *a_len; ++i) {
        int av = (int)a[i] - borrow;
        int bv = (i < b_len) ? (int)b[i] : 0;
        int diff = av - bv;

        if (diff < 0) {
            diff += 256;
            borrow = 1;
        } else {
            borrow = 0;
        }

        a[i] = (uint8_t)diff;
    }

    normalize_le(a, a_len);
}

static int shift_left1_le(uint8_t **v, size_t *len, size_t *cap) {
    size_t i;
    uint16_t carry = 0u;

    for (i = 0u; i < *len; ++i) {
        uint16_t value = (uint16_t)((uint16_t)(*v)[i] << 1u) | carry;
        (*v)[i] = (uint8_t)(value & 0xFFu);
        carry = (uint16_t)(value >> 8u);
    }

    if (carry != 0u) {
        if (!ensure_capacity(v, cap, *len + 1u)) {
            return 0;
        }
        (*v)[*len] = (uint8_t)carry;
        *len += 1u;
    }

    return 1;
}

static int add_bit_le(uint8_t **v, size_t *len, size_t *cap, uint8_t bit) {
    size_t i;
    uint16_t carry;

    if (bit == 0u) {
        return 1;
    }

    carry = 1u;
    for (i = 0u; i < *len && carry != 0u; ++i) {
        uint16_t value = (uint16_t)(*v)[i] + carry;
        (*v)[i] = (uint8_t)(value & 0xFFu);
        carry = (uint16_t)(value >> 8u);
    }

    if (carry != 0u) {
        if (!ensure_capacity(v, cap, *len + 1u)) {
            return 0;
        }
        (*v)[*len] = (uint8_t)carry;
        *len += 1u;
    }

    return 1;
}

static mithril_status mod_reduce_be(
    const uint8_t *dividend_be,
    size_t dividend_len,
    const uint8_t *modulus_be,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    uint8_t *mod_le = NULL;
    size_t mod_le_len = 0u;
    uint8_t *rem_le = NULL;
    size_t rem_len = 1u;
    size_t rem_cap = 1u;
    size_t i;

    if (modulus_len == 0u || (modulus_len == 1u && modulus_be[0] == 0u)) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (!be_to_le_alloc(modulus_be, modulus_len, &mod_le, &mod_le_len)) {
        return MITHRIL_ERR_INTERNAL;
    }

    rem_le = (uint8_t *)calloc(rem_cap, 1u);
    if (rem_le == NULL) {
        free(mod_le);
        return MITHRIL_ERR_INTERNAL;
    }

    for (i = 0u; i < dividend_len; ++i) {
        uint8_t byte = dividend_be[i];
        int bit;

        for (bit = 7; bit >= 0; --bit) {
            uint8_t in_bit = (uint8_t)((byte >> (uint8_t)bit) & 0x01u);

            if (!shift_left1_le(&rem_le, &rem_len, &rem_cap)) {
                free(mod_le);
                free(rem_le);
                return MITHRIL_ERR_INTERNAL;
            }

            if (!add_bit_le(&rem_le, &rem_len, &rem_cap, in_bit)) {
                free(mod_le);
                free(rem_le);
                return MITHRIL_ERR_INTERNAL;
            }

            if (cmp_le(rem_le, rem_len, mod_le, mod_le_len) >= 0) {
                sub_le_inplace(rem_le, &rem_len, mod_le, mod_le_len);
            }
        }
    }

    normalize_le(rem_le, &rem_len);

    if (out_len < rem_len) {
        free(mod_le);
        free(rem_le);
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < rem_len; ++i) {
        out[i] = rem_le[rem_len - 1u - i];
    }
    *written_len = rem_len;

    free(mod_le);
    free(rem_le);
    return MITHRIL_OK;
}

mithril_status mithril_provider_c23_modarith_add_mod(
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
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;
    uint8_t *sum_buf = NULL;
    size_t sum_written = 0u;
    size_t sum_cap;
    mithril_status status;

    if (state == 0 || a == 0 || b == 0 || modulus == 0 || out == 0 || written_len == 0 || a_len == 0u || b_len == 0u || modulus_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    modulus = skip_leading_zeros_be(modulus, &modulus_len);
    if (modulus_len == 1u && modulus[0] == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    sum_cap = ((a_len > b_len) ? a_len : b_len) + 1u;
    sum_buf = (uint8_t *)calloc(sum_cap, 1u);
    if (sum_buf == NULL) {
        return MITHRIL_ERR_INTERNAL;
    }

    status = mithril_provider_c23_bigint_add(user_data, a, a_len, b, b_len, sum_buf, sum_cap, &sum_written);
    if (status != MITHRIL_OK) {
        free(sum_buf);
        return status;
    }

    status = mod_reduce_be(sum_buf, sum_written, modulus, modulus_len, out, out_len, written_len);
    free(sum_buf);
    return status;
}

mithril_status mithril_provider_c23_modarith_mul_mod(
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
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;
    uint8_t *mul_buf = NULL;
    size_t mul_written = 0u;
    size_t mul_cap;
    mithril_status status;

    if (state == 0 || a == 0 || b == 0 || modulus == 0 || out == 0 || written_len == 0 || a_len == 0u || b_len == 0u || modulus_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    modulus = skip_leading_zeros_be(modulus, &modulus_len);
    if (modulus_len == 1u && modulus[0] == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    mul_cap = a_len + b_len;
    if (mul_cap == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    mul_buf = (uint8_t *)calloc(mul_cap, 1u);
    if (mul_buf == NULL) {
        return MITHRIL_ERR_INTERNAL;
    }

    status = mithril_provider_c23_bigint_mul(user_data, a, a_len, b, b_len, mul_buf, mul_cap, &mul_written);
    if (status != MITHRIL_OK) {
        free(mul_buf);
        return status;
    }

    status = mod_reduce_be(mul_buf, mul_written, modulus, modulus_len, out, out_len, written_len);
    free(mul_buf);
    return status;
}
