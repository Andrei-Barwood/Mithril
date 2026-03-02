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

static int cmp_be(const uint8_t *a, size_t a_len, const uint8_t *b, size_t b_len) {
    size_t i;

    if (a_len < b_len) {
        return -1;
    }
    if (a_len > b_len) {
        return 1;
    }

    for (i = 0u; i < a_len; ++i) {
        if (a[i] < b[i]) {
            return -1;
        }
        if (a[i] > b[i]) {
            return 1;
        }
    }

    return 0;
}

static mithril_status bigint_add_be(
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    size_t i = 0u;
    size_t ia;
    size_t ib;
    size_t io;
    uint16_t carry = 0u;
    size_t need_len;

    need_len = ((a_len > b_len) ? a_len : b_len) + 1u;
    if (out_len < need_len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < need_len; ++i) {
        out[i] = 0u;
    }

    ia = a_len;
    ib = b_len;
    io = need_len;

    while (ia > 0u || ib > 0u || carry > 0u) {
        uint16_t av = 0u;
        uint16_t bv = 0u;
        uint16_t sum;

        if (io == 0u) {
            return MITHRIL_ERR_INVALID_ARGUMENT;
        }

        if (ia > 0u) {
            --ia;
            av = a[ia];
        }
        if (ib > 0u) {
            --ib;
            bv = b[ib];
        }

        --io;
        sum = (uint16_t)(av + bv + carry);
        out[io] = (uint8_t)(sum & 0xFFu);
        carry = (uint16_t)(sum >> 8u);
    }

    while (io + 1u < need_len && out[io] == 0u) {
        ++io;
    }

    *written_len = need_len - io;
    if (io > 0u) {
        size_t k;
        for (k = 0u; k < *written_len; ++k) {
            out[k] = out[io + k];
        }
    }

    return MITHRIL_OK;
}

static mithril_status bigint_sub_be(
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    size_t ia;
    size_t ib;
    size_t io;
    int borrow = 0;
    size_t i;

    if (cmp_be(a, a_len, b, b_len) < 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (out_len < a_len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < a_len; ++i) {
        out[i] = 0u;
    }

    ia = a_len;
    ib = b_len;
    io = a_len;

    while (ia > 0u) {
        int av;
        int bv = 0;
        int diff;

        --ia;
        --io;
        av = (int)a[ia] - borrow;
        if (ib > 0u) {
            --ib;
            bv = (int)b[ib];
        }

        diff = av - bv;
        if (diff < 0) {
            diff += 256;
            borrow = 1;
        } else {
            borrow = 0;
        }

        out[io] = (uint8_t)diff;
    }

    if (borrow != 0) {
        return MITHRIL_ERR_INTERNAL;
    }

    io = 0u;
    while (io + 1u < a_len && out[io] == 0u) {
        ++io;
    }

    *written_len = a_len - io;
    if (io > 0u) {
        size_t k;
        for (k = 0u; k < *written_len; ++k) {
            out[k] = out[io + k];
        }
    }

    return MITHRIL_OK;
}

static mithril_status bigint_mul_be(
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    size_t need_len;
    size_t i;
    size_t j;

    need_len = a_len + b_len;
    if (need_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
    if (out_len < need_len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < need_len; ++i) {
        out[i] = 0u;
    }

    for (i = a_len; i > 0u; --i) {
        uint16_t carry = 0u;
        size_t ai = i - 1u;

        for (j = b_len; j > 0u; --j) {
            size_t bj = j - 1u;
            size_t k = ai + bj + 1u;
            uint32_t prod = (uint32_t)a[ai] * (uint32_t)b[bj] + (uint32_t)out[k] + (uint32_t)carry;
            out[k] = (uint8_t)(prod & 0xFFu);
            carry = (uint16_t)(prod >> 8u);
        }

        out[ai] = (uint8_t)((uint16_t)out[ai] + carry);
    }

    i = 0u;
    while (i + 1u < need_len && out[i] == 0u) {
        ++i;
    }

    *written_len = need_len - i;
    if (i > 0u) {
        size_t k;
        for (k = 0u; k < *written_len; ++k) {
            out[k] = out[i + k];
        }
    }

    return MITHRIL_OK;
}

mithril_status mithril_provider_c23_on_activate(void *user_data) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;
    if (state == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
    state->initialized = 1;
    return MITHRIL_OK;
}

void mithril_provider_c23_on_deactivate(void *user_data) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;
    if (state != 0) {
        state->initialized = 0;
    }
}

mithril_status mithril_provider_c23_get_capabilities(void *user_data, mithril_capabilities *out_caps) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;
    if (state == 0 || out_caps == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    out_caps->feature_flags = MITHRIL_CAP_BIGINT | MITHRIL_CAP_MODARITH;
    out_caps->provider_count = 1u;
    out_caps->reserved = 0u;
    return MITHRIL_OK;
}

mithril_status mithril_provider_c23_bigint_add(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;

    if (state == 0 || a == 0 || b == 0 || out == 0 || written_len == 0 || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    a = skip_leading_zeros_be(a, &a_len);
    b = skip_leading_zeros_be(b, &b_len);

    return bigint_add_be(a, a_len, b, b_len, out, out_len, written_len);
}

mithril_status mithril_provider_c23_bigint_sub(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;

    if (state == 0 || a == 0 || b == 0 || out == 0 || written_len == 0 || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    a = skip_leading_zeros_be(a, &a_len);
    b = skip_leading_zeros_be(b, &b_len);

    return bigint_sub_be(a, a_len, b, b_len, out, out_len, written_len);
}

mithril_status mithril_provider_c23_bigint_mul(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    mithril_provider_c23_state *state = (mithril_provider_c23_state *)user_data;

    if (state == 0 || a == 0 || b == 0 || out == 0 || written_len == 0 || a_len == 0u || b_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    a = skip_leading_zeros_be(a, &a_len);
    b = skip_leading_zeros_be(b, &b_len);

    return bigint_mul_be(a, a_len, b, b_len, out, out_len, written_len);
}
