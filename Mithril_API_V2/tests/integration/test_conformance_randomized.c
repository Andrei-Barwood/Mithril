#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_modarith.h"
#include "mithril/mithril_provider.h"

#ifndef MITHRIL_CONFORMANCE_CASES_PER_OPERATION
#define MITHRIL_CONFORMANCE_CASES_PER_OPERATION 1200
#endif

#if MITHRIL_CONFORMANCE_CASES_PER_OPERATION <= 0
#error "MITHRIL_CONFORMANCE_CASES_PER_OPERATION must be a positive integer."
#endif

enum {
    CASES_PER_OPERATION = MITHRIL_CONFORMANCE_CASES_PER_OPERATION,
    MAX_INPUT_BYTES = 64,
    MAX_OUTPUT_BYTES = 256
};

typedef enum conformance_op {
    OP_BIGINT_ADD = 0,
    OP_BIGINT_SUB = 1,
    OP_BIGINT_MUL = 2,
    OP_MODARITH_ADD_MOD = 3,
    OP_MODARITH_MUL_MOD = 4
} conformance_op;

typedef struct prng_state {
    uint64_t x;
} prng_state;

static void expect_ok(mithril_status status, const char *msg) {
    if (status != MITHRIL_OK) {
        fprintf(stderr, "[FAIL] %s: status=%d\n", msg, (int)status);
        assert(status == MITHRIL_OK);
    }
}

static uint64_t prng_next_u64(prng_state *st) {
    uint64_t x = st->x;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    st->x = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

static int should_inject_invalid_case(prng_state *st) {
    return ((prng_next_u64(st) & UINT64_C(0x7)) == 0u) ? 1 : 0;
}

static size_t prng_next_len(prng_state *st, size_t min_len, size_t max_len) {
    uint64_t span = (uint64_t)(max_len - min_len + 1u);
    return min_len + (size_t)(prng_next_u64(st) % span);
}

static void prng_fill_bytes(prng_state *st, uint8_t *out, size_t out_len) {
    size_t i = 0u;
    while (i < out_len) {
        uint64_t v = prng_next_u64(st);
        size_t j;
        for (j = 0u; j < 8u && i < out_len; ++j, ++i) {
            out[i] = (uint8_t)((v >> (j * 8u)) & 0xFFu);
        }
    }
}

static const uint8_t *skip_leading_zeros(const uint8_t *in, size_t *len) {
    size_t i = 0u;
    while (i + 1u < *len && in[i] == 0u) {
        ++i;
    }
    *len -= i;
    return in + i;
}

static int cmp_be(const uint8_t *a, size_t a_len, const uint8_t *b, size_t b_len) {
    size_t i;
    a = skip_leading_zeros(a, &a_len);
    b = skip_leading_zeros(b, &b_len);

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

static int all_zero(const uint8_t *in, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (in[i] != 0u) {
            return 0;
        }
    }
    return 1;
}

static void swap_bytes(uint8_t *a, size_t *a_len, uint8_t *b, size_t *b_len) {
    uint8_t tmp[MAX_INPUT_BYTES];
    size_t tmp_len = *a_len;
    size_t i;

    assert(*a_len <= MAX_INPUT_BYTES);
    assert(*b_len <= MAX_INPUT_BYTES);

    for (i = 0u; i < *a_len; ++i) {
        tmp[i] = a[i];
    }
    for (i = 0u; i < *b_len; ++i) {
        a[i] = b[i];
    }
    for (i = 0u; i < tmp_len; ++i) {
        b[i] = tmp[i];
    }

    *a_len = *b_len;
    *b_len = tmp_len;
}

static mithril_status run_operation(
    conformance_op op,
    mithril_context *ctx,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len) {
    switch (op) {
        case OP_BIGINT_ADD:
            return mithril_bigint_add(ctx, a, a_len, b, b_len, out, out_len, written_len);
        case OP_BIGINT_SUB:
            return mithril_bigint_sub(ctx, a, a_len, b, b_len, out, out_len, written_len);
        case OP_BIGINT_MUL:
            return mithril_bigint_mul(ctx, a, a_len, b, b_len, out, out_len, written_len);
        case OP_MODARITH_ADD_MOD:
            return mithril_modarith_add_mod(ctx, a, a_len, b, b_len, modulus, modulus_len, out, out_len, written_len);
        case OP_MODARITH_MUL_MOD:
            return mithril_modarith_mul_mod(ctx, a, a_len, b, b_len, modulus, modulus_len, out, out_len, written_len);
    }
    return MITHRIL_ERR_INTERNAL;
}

static const char *op_name(conformance_op op) {
    switch (op) {
        case OP_BIGINT_ADD:
            return "bigint_add";
        case OP_BIGINT_SUB:
            return "bigint_sub";
        case OP_BIGINT_MUL:
            return "bigint_mul";
        case OP_MODARITH_ADD_MOD:
            return "modarith_add_mod";
        case OP_MODARITH_MUL_MOD:
            return "modarith_mul_mod";
    }
    return "unknown";
}

static uint32_t run_conformance_for_op(
    conformance_op op,
    mithril_context *ctx_c23,
    mithril_context *ctx_flint,
    uint64_t seed) {
    prng_state st;
    uint32_t i;
    uint32_t divergences = 0u;
    uint32_t error_cases = 0u;
    uint32_t error_divergences = 0u;

    st.x = seed;

    for (i = 0u; i < CASES_PER_OPERATION; ++i) {
        uint8_t a[MAX_INPUT_BYTES];
        uint8_t b[MAX_INPUT_BYTES];
        uint8_t m[MAX_INPUT_BYTES];
        uint8_t out_c23[MAX_OUTPUT_BYTES];
        uint8_t out_flint[MAX_OUTPUT_BYTES];
        size_t a_len = prng_next_len(&st, 1u, MAX_INPUT_BYTES);
        size_t b_len = prng_next_len(&st, 1u, MAX_INPUT_BYTES);
        size_t m_len = prng_next_len(&st, 1u, MAX_INPUT_BYTES);
        size_t written_c23 = ~(size_t)0u;
        size_t written_flint = ~((size_t)1u);
        mithril_status st_c23;
        mithril_status st_flint;
        int inject_invalid = should_inject_invalid_case(&st);

        prng_fill_bytes(&st, a, a_len);
        prng_fill_bytes(&st, b, b_len);
        prng_fill_bytes(&st, m, m_len);

        if (inject_invalid) {
            if (op == OP_MODARITH_ADD_MOD || op == OP_MODARITH_MUL_MOD) {
                m_len = 1u;
                m[0] = 0u;
            } else {
                a_len = 0u;
            }
        } else {
            if (op == OP_BIGINT_SUB && cmp_be(a, a_len, b, b_len) < 0) {
                swap_bytes(a, &a_len, b, &b_len);
            }

            if ((op == OP_MODARITH_ADD_MOD || op == OP_MODARITH_MUL_MOD) && all_zero(m, m_len)) {
                m[m_len - 1u] = 1u;
            }
        }

        st_c23 = run_operation(
            op, ctx_c23, a, a_len, b, b_len, m, m_len, out_c23, sizeof(out_c23), &written_c23);
        st_flint = run_operation(
            op, ctx_flint, a, a_len, b, b_len, m, m_len, out_flint, sizeof(out_flint), &written_flint);

        if (st_c23 != MITHRIL_OK || st_flint != MITHRIL_OK) {
            ++error_cases;
            if (st_c23 != st_flint) {
                ++divergences;
                ++error_divergences;
                if (divergences <= 10u) {
                    fprintf(
                        stderr,
                        "[DIVERGENCE] op=%s case=%u invalid=%d status(c23=%d flint=%d)\n",
                        op_name(op),
                        i,
                        inject_invalid,
                        (int)st_c23,
                        (int)st_flint);
                }
            }
            continue;
        }

        if (written_c23 != written_flint || memcmp(out_c23, out_flint, written_c23) != 0) {
            ++divergences;
            if (divergences <= 10u) {
                fprintf(
                    stderr,
                    "[DIVERGENCE] op=%s case=%u invalid=%d status(c23=%d flint=%d) written(c23=%zu flint=%zu)\n",
                    op_name(op),
                    i,
                    inject_invalid,
                    (int)st_c23,
                    (int)st_flint,
                    written_c23,
                    written_flint);
            }
        }
    }

    printf(
        "[CONFORMANCE] op=%s seed=0x%016llX cases=%u error_cases=%u divergences=%u error_divergences=%u\n",
        op_name(op),
        (unsigned long long)seed,
        (unsigned)CASES_PER_OPERATION,
        (unsigned)error_cases,
        (unsigned)divergences,
        (unsigned)error_divergences);

    return divergences;
}

int main(void) {
    const uint64_t fixed_seeds[] = {
        UINT64_C(0xC23F1A77A55EED11),
        UINT64_C(0xF11E9B0A33D4C2E7),
        UINT64_C(0x5A17D0C4B88E7123)
    };
    const conformance_op ops[] = {
        OP_BIGINT_ADD,
        OP_BIGINT_SUB,
        OP_BIGINT_MUL,
        OP_MODARITH_ADD_MOD,
        OP_MODARITH_MUL_MOD
    };
    mithril_context *ctx_c23 = NULL;
    mithril_context *ctx_flint = NULL;
    uint32_t total_divergences = 0u;
    size_t i;
    size_t s;
    mithril_status st;

    st = mithril_init(&ctx_c23, NULL);
    expect_ok(st, "mithril_init c23 context");
    st = mithril_provider_activate(ctx_c23, "c23");
    expect_ok(st, "activate c23 context");

    st = mithril_init(&ctx_flint, NULL);
    expect_ok(st, "mithril_init flint context");
    st = mithril_provider_activate(ctx_flint, "flint");
    expect_ok(st, "activate flint context");

    for (s = 0u; s < (sizeof(fixed_seeds) / sizeof(fixed_seeds[0])); ++s) {
        for (i = 0u; i < (sizeof(ops) / sizeof(ops[0])); ++i) {
            uint64_t op_seed = fixed_seeds[s] ^ (UINT64_C(0x9E3779B97F4A7C15) * (uint64_t)(i + 1u));
            total_divergences += run_conformance_for_op(ops[i], ctx_c23, ctx_flint, op_seed);
        }
    }

    mithril_shutdown(ctx_c23);
    mithril_shutdown(ctx_flint);

    printf(
        "[CONFORMANCE] summary cases_per_operation=%u operations=%u seeds=%u total_cases=%u total_divergences=%u\n",
        (unsigned)CASES_PER_OPERATION,
        (unsigned)(sizeof(ops) / sizeof(ops[0])),
        (unsigned)(sizeof(fixed_seeds) / sizeof(fixed_seeds[0])),
        (unsigned)(CASES_PER_OPERATION * (sizeof(ops) / sizeof(ops[0])) * (sizeof(fixed_seeds) / sizeof(fixed_seeds[0]))),
        (unsigned)total_divergences);

    if (total_divergences != 0u) {
        return 1;
    }

    puts("[OK] test_conformance_randomized");
    return 0;
}
