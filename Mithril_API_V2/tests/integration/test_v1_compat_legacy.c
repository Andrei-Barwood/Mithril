#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/compat/mithril_v1_compat.h"

#if defined(MITHRIL_USE_FLINT)
#include <flint/fmpz.h>
#endif

static const uint8_t g_ed25519_l_le[MITHRIL_SCALAR_BYTES] = {
    0xEDu, 0xD3u, 0xF5u, 0x5Cu, 0x1Au, 0x63u, 0x12u, 0x58u,
    0xD6u, 0x9Cu, 0xF7u, 0xA2u, 0xDEu, 0xF9u, 0xDEu, 0x14u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x10u
};

static uint32_t g_prng_state = 0x7E57C0DEu;

static uint32_t xorshift32(void) {
    uint32_t x = g_prng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_prng_state = x;
    return x;
}

static uint8_t next_byte(void) {
    return (uint8_t)(xorshift32() & 0xFFu);
}

static void fill_random(uint8_t *out, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        out[i] = next_byte();
    }
}

static int ref_cmp_le(const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;
    for (i = len; i > 0u; --i) {
        if (a[i - 1u] < b[i - 1u]) {
            return -1;
        }
        if (a[i - 1u] > b[i - 1u]) {
            return 1;
        }
    }
    return 0;
}

static int ref_add_le(uint8_t *out, const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;
    uint16_t carry = 0u;

    for (i = 0u; i < len; ++i) {
        uint16_t sum = (uint16_t)a[i] + (uint16_t)b[i] + carry;
        out[i] = (uint8_t)(sum & 0xFFu);
        carry = (uint16_t)(sum >> 8u);
    }

    return (carry != 0u) ? 1 : 0;
}

static void ref_sub_le(uint8_t *out, const uint8_t *a, const uint8_t *b, size_t len) {
    size_t i;
    int borrow = 0;

    for (i = 0u; i < len; ++i) {
        int diff = (int)a[i] - (int)b[i] - borrow;
        if (diff < 0) {
            diff += 256;
            borrow = 1;
        } else {
            borrow = 0;
        }
        out[i] = (uint8_t)diff;
    }
}

static void ref_sub_one_le(uint8_t *inout, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (inout[i] > 0u) {
            inout[i] = (uint8_t)(inout[i] - 1u);
            return;
        }
        inout[i] = 0xFFu;
    }
}

static void expect_eq_bytes(const uint8_t *got, const uint8_t *expected, size_t len, const char *msg) {
    if (memcmp(got, expected, len) != 0) {
        size_t i;
        fprintf(stderr, "[FAIL] %s\nexpected:", msg);
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", expected[i]);
        }
        fprintf(stderr, "\ngot:     ");
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", got[i]);
        }
        fprintf(stderr, "\n");
        assert(0 && "bytes mismatch");
    }
}

static void test_lifecycle_idempotent(void) {
    assert(mithril_v1_compat_init() == 0);
    assert(mithril_v1_compat_init() == 0);
    mithril_v1_compat_shutdown();
    mithril_v1_compat_shutdown();
    assert(mithril_v1_compat_init() == 0);
}

static void test_invalid_arguments(void) {
    uint8_t out[MITHRIL_SCALAR_BYTES];
    uint8_t in[MITHRIL_SCALAR_BYTES];
    memset(out, 0, sizeof(out));
    memset(in, 0, sizeof(in));

    assert(mithril_add_scalar(NULL, in, in) == -1);
    assert(mithril_add_scalar(out, NULL, in) == -1);
    assert(mithril_add_scalar(out, in, NULL) == -1);
    assert(mithril_add_mod_l(NULL, in, in) == -1);
    assert(mithril_add_mod_l(out, NULL, in) == -1);
    assert(mithril_add_mod_l(out, in, NULL) == -1);
    assert(mithril_add_constant_time(NULL, in, in, sizeof(in)) == -1);
    assert(mithril_add_constant_time(out, NULL, in, sizeof(in)) == -1);
    assert(mithril_add_constant_time(out, in, NULL, sizeof(in)) == -1);
    assert(mithril_add_constant_time(out, in, in, 0u) == -1);
    assert(sodium_sub_with_underflow(NULL, in, in, sizeof(in)) == -1);
    assert(sodium_sub_with_underflow(out, NULL, in, sizeof(in)) == -1);
    assert(sodium_sub_with_underflow(out, in, NULL, sizeof(in)) == -1);
    assert(sodium_sub_with_underflow(out, in, in, 0u) == -1);
}

static void test_add_scalar_randomized(void) {
    uint8_t a[MITHRIL_SCALAR_BYTES];
    uint8_t b[MITHRIL_SCALAR_BYTES];
    uint8_t got[MITHRIL_SCALAR_BYTES];
    uint8_t expected[MITHRIL_SCALAR_BYTES];
    int i;

    for (i = 0; i < 1024; ++i) {
        int rc_got;
        int rc_expected;

        fill_random(a, sizeof(a));
        fill_random(b, sizeof(b));

        rc_got = mithril_add_scalar(got, a, b);
        rc_expected = ref_add_le(expected, a, b, sizeof(expected));

        assert(rc_got == rc_expected);
        expect_eq_bytes(got, expected, sizeof(got), "add_scalar randomized");
    }
}

static void test_add_constant_time_randomized(void) {
    uint8_t a[64];
    uint8_t b[64];
    uint8_t got[64];
    uint8_t expected[64];
    int i;

    for (i = 0; i < 1024; ++i) {
        size_t len = (size_t)((xorshift32() % 64u) + 1u);
        int rc_got;
        int rc_expected;

        fill_random(a, len);
        fill_random(b, len);

        rc_got = mithril_add_constant_time(got, a, b, len);
        rc_expected = ref_add_le(expected, a, b, len);

        assert(rc_got == rc_expected);
        expect_eq_bytes(got, expected, len, "add_constant_time randomized");
    }
}

static void test_sub_underflow_randomized(void) {
    uint8_t a[64];
    uint8_t b[64];
    uint8_t got[64];
    uint8_t expected[64];
    int i;

    for (i = 0; i < 1024; ++i) {
        size_t len = (size_t)((xorshift32() % 64u) + 1u);
        int cmp;
        int rc_got;
        int rc_expected;

        fill_random(a, len);
        fill_random(b, len);

        rc_got = sodium_sub_with_underflow(got, a, b, len);
        cmp = ref_cmp_le(a, b, len);

        if (cmp >= 0) {
            ref_sub_le(expected, a, b, len);
            rc_expected = E_SODIUM_OK;
        } else {
            ref_sub_le(expected, b, a, len);
            ref_sub_one_le(expected, len);
            rc_expected = E_SODIUM_UFL;
        }

        assert(rc_got == rc_expected);
        expect_eq_bytes(got, expected, len, "sub_with_underflow randomized");
    }
}

static void sub_one_le_32(uint8_t out[MITHRIL_SCALAR_BYTES], const uint8_t in[MITHRIL_SCALAR_BYTES]) {
    size_t i;
    memcpy(out, in, MITHRIL_SCALAR_BYTES);
    for (i = 0u; i < MITHRIL_SCALAR_BYTES; ++i) {
        if (out[i] > 0u) {
            out[i] = (uint8_t)(out[i] - 1u);
            return;
        }
        out[i] = 0xFFu;
    }
}

static void test_add_mod_l_vectors(void) {
    uint8_t a[MITHRIL_SCALAR_BYTES];
    uint8_t b[MITHRIL_SCALAR_BYTES];
    uint8_t out[MITHRIL_SCALAR_BYTES];
    uint8_t expected[MITHRIL_SCALAR_BYTES];
    uint8_t l_minus_one[MITHRIL_SCALAR_BYTES];

    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    memset(expected, 0, sizeof(expected));

    a[0] = 1u;
    b[0] = 2u;
    expected[0] = 3u;
    assert(mithril_add_mod_l(out, a, b) == 0);
    expect_eq_bytes(out, expected, sizeof(out), "add_mod_l small values");

    sub_one_le_32(l_minus_one, g_ed25519_l_le);
    memset(b, 0, sizeof(b));
    b[0] = 1u;
    memset(expected, 0, sizeof(expected));
    assert(mithril_add_mod_l(out, l_minus_one, b) == 0);
    expect_eq_bytes(out, expected, sizeof(out), "add_mod_l (L-1)+1");

    memset(expected, 0, sizeof(expected));
    sub_one_le_32(expected, g_ed25519_l_le);
    sub_one_le_32(expected, expected);
    assert(mithril_add_mod_l(out, l_minus_one, l_minus_one) == 0);
    expect_eq_bytes(out, expected, sizeof(out), "add_mod_l (L-1)+(L-1)");
}

#if defined(MITHRIL_USE_FLINT)
static void test_fmpz_wrappers_extended(void) {
    fmpz_t a;
    fmpz_t b;
    fmpz_t result;
    int rc;

    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(result);

    fmpz_zero(a);
    fmpz_set_ui(b, 123456u);
    rc = fmpz_mul_safe(result, a, b);
    assert(rc == 0);
    assert(fmpz_is_zero(result));

    fmpz_set_ui(a, 321u);
    rc = fmpz_square_safe(result, a);
    assert(rc == 0);
    assert(fmpz_cmp_ui(result, 103041u) == 0);

    fmpz_set_si(a, -5);
    fmpz_set_ui(b, 3u);
    rc = fmpz_mul_safe(result, a, b);
    assert(rc == -1);

    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(result);
}
#endif

int main(void) {
    test_lifecycle_idempotent();
    test_invalid_arguments();
    test_add_scalar_randomized();
    test_add_constant_time_randomized();
    test_sub_underflow_randomized();
    test_add_mod_l_vectors();
#if defined(MITHRIL_USE_FLINT)
    test_fmpz_wrappers_extended();
#endif

    mithril_v1_compat_shutdown();
    puts("[OK] test_v1_compat_legacy");
    return 0;
}
