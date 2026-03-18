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

static void expect_zeroes(const uint8_t *value, size_t len, const char *msg) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        if (value[i] != 0u) {
            fprintf(stderr, "[FAIL] %s at byte %zu -> %02X\n", msg, i, value[i]);
            assert(0 && "non-zero byte");
        }
    }
}

static void test_add_scalar_overflow(void) {
    uint8_t a[MITHRIL_SCALAR_BYTES];
    uint8_t b[MITHRIL_SCALAR_BYTES];
    uint8_t out[MITHRIL_SCALAR_BYTES];
    int rc;

    memset(a, 0xFF, sizeof(a));
    memset(b, 0x00, sizeof(b));
    b[0] = 0x01u;

    rc = mithril_add_scalar(out, a, b);
    assert(rc == 1);
    expect_zeroes(out, sizeof(out), "add_scalar overflow wraps to zero");
}

static void test_add_mod_l_wrap(void) {
    uint8_t a[MITHRIL_SCALAR_BYTES];
    uint8_t b[MITHRIL_SCALAR_BYTES];
    uint8_t out[MITHRIL_SCALAR_BYTES];
    size_t i;
    int rc;

    memcpy(a, g_ed25519_l_le, sizeof(a));
    for (i = 0u; i < sizeof(a); ++i) {
        if (a[i] > 0u) {
            a[i] = (uint8_t)(a[i] - 1u);
            break;
        }
        a[i] = 0xFFu;
    }

    memset(b, 0x00, sizeof(b));
    b[0] = 0x01u;

    rc = mithril_add_mod_l(out, a, b);
    assert(rc == 0);
    expect_zeroes(out, sizeof(out), "add_mod_l (L-1)+1 == 0");
}

static void test_add_constant_time_contract(void) {
    uint8_t out[1] = {0};
    const uint8_t a[1] = {0xFFu};
    const uint8_t b[1] = {0x01u};
    int rc;

    rc = mithril_add_constant_time(out, a, b, sizeof(out));
    assert(rc == 1);
    assert(out[0] == 0x00u);
}

static void test_sub_with_underflow_contract(void) {
    const unsigned char a1[4] = {0x05u, 0x00u, 0x00u, 0x00u};
    const unsigned char b1[4] = {0x03u, 0x00u, 0x00u, 0x00u};
    const unsigned char a2[4] = {0x03u, 0x00u, 0x00u, 0x00u};
    const unsigned char b2[4] = {0x05u, 0x00u, 0x00u, 0x00u};
    unsigned char out[4];
    int rc;

    memset(out, 0x00, sizeof(out));
    rc = sodium_sub_with_underflow(out, a1, b1, sizeof(out));
    assert(rc == E_SODIUM_OK);
    assert(out[0] == 0x02u);
    assert(out[1] == 0x00u);
    assert(out[2] == 0x00u);
    assert(out[3] == 0x00u);

    memset(out, 0x00, sizeof(out));
    rc = sodium_sub_with_underflow(out, a2, b2, sizeof(out));
    assert(rc == E_SODIUM_UFL);
    assert(out[0] == 0x01u);
    assert(out[1] == 0x00u);
    assert(out[2] == 0x00u);
    assert(out[3] == 0x00u);
}

#if defined(MITHRIL_USE_FLINT)
static void test_fmpz_wrappers(void) {
    fmpz_t a;
    fmpz_t b;
    fmpz_t result;
    int rc;

    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(result);

    fmpz_set_ui(a, 12345u);
    fmpz_set_ui(b, 678u);

    rc = fmpz_mul_safe(result, a, b);
    assert(rc == 0);
    assert(fmpz_cmp_ui(result, 8369910u) == 0);

    rc = fmpz_square_safe(result, a);
    assert(rc == 0);
    assert(fmpz_cmp_ui(result, 152399025u) == 0);

    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(result);
}
#endif

int main(void) {
    assert(mithril_v1_compat_init() == 0);

    test_add_scalar_overflow();
    test_add_mod_l_wrap();
    test_add_constant_time_contract();
    test_sub_with_underflow_contract();
#if defined(MITHRIL_USE_FLINT)
    test_fmpz_wrappers();
#endif

    mithril_v1_compat_shutdown();
    puts("[OK] test_v1_compat_smoke");
    return 0;
}
