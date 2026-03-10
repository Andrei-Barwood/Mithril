#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "mithril/compat/mithril_v1_compat.h"

#if defined(MITHRIL_USE_FLINT)
#include <gmp.h>
#include <flint/flint.h>
#include <flint/fmpz.h>

static void test_div_fmpz_basic(void) {
    fmpz_t dividend;
    fmpz_t divisor;
    fmpz_t q;
    fmpz_t r;
    int rc;

    fmpz_init(dividend);
    fmpz_init(divisor);
    fmpz_init(q);
    fmpz_init(r);

    fmpz_set_si(dividend, 100);
    fmpz_set_si(divisor, 7);
    rc = div_fmpz(q, r, dividend, divisor);
    assert(rc == 0);
    assert(fmpz_cmp_si(q, 14) == 0);
    assert(fmpz_cmp_si(r, 2) == 0);

    fmpz_set_si(dividend, -100);
    fmpz_set_si(divisor, 7);
    rc = div_fmpz(q, r, dividend, divisor);
    assert(rc == 0);
    assert(fmpz_cmp_si(q, -14) == 0);
    assert(fmpz_cmp_si(r, -2) == 0);

    fmpz_zero(divisor);
    rc = div_fmpz(q, r, dividend, divisor);
    assert(rc == -1);

    fmpz_clear(dividend);
    fmpz_clear(divisor);
    fmpz_clear(q);
    fmpz_clear(r);
}

static void test_rem_mod_pow_of_2_vectors(void) {
    fmpz_t x;
    fmpz_t r;

    fmpz_init(x);
    fmpz_init(r);

    fmpz_set_ui(x, 0x12345u);
    rem_mod_pow_of_2(x, 8u, r);
    assert(fmpz_cmp_ui(r, 0x45u) == 0);

    rem_mod_pow_of_2(x, 0u, r);
    assert(fmpz_is_zero(r));

    fmpz_set_si(x, -1);
    rem_mod_pow_of_2(x, 8u, r);
    assert(fmpz_cmp_ui(r, 255u) == 0);

    fmpz_clear(x);
    fmpz_clear(r);
}

static void test_flint_kmul_equivalence(void) {
    fmpz_t a;
    fmpz_t b;
    fmpz_t got;
    fmpz_t expected;

    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(got);
    fmpz_init(expected);

    fmpz_set_ui(a, 123456789u);
    fmpz_set_ui(b, 987654321u);

    flint_kmul(a, b, got);
    fmpz_mul(expected, a, b);
    assert(fmpz_equal(got, expected));

    fmpz_set_si(a, -314159u);
    fmpz_set_ui(b, 271828u);

    flint_kmul(a, b, got);
    fmpz_mul(expected, a, b);
    assert(fmpz_equal(got, expected));

    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(got);
    fmpz_clear(expected);
}

static void test_flint_kmul_limbs_equivalence(void) {
    fmpz_t got;
    fmpz_t expected;
    fmpz_t a;
    fmpz_t b;
    mp_limb_t a_limbs[3];
    mp_limb_t b_limbs[2];

    fmpz_init(got);
    fmpz_init(expected);
    fmpz_init(a);
    fmpz_init(b);

    a_limbs[0] = (mp_limb_t)0x0123456789ABCDEFuLL;
    a_limbs[1] = (mp_limb_t)0x0F0E0D0C0B0A0908uLL;
    a_limbs[2] = (mp_limb_t)0x0000000000000007uLL;

    b_limbs[0] = (mp_limb_t)0x1111111111111111uLL;
    b_limbs[1] = (mp_limb_t)0x0000000000000003uLL;

    flint_kmul_limbs(a_limbs, 3, b_limbs, 2, got);

    fmpz_set_ui(a, 0u);
    fmpz_add_ui(a, a, (ulong)a_limbs[2]);
    fmpz_mul_2exp(a, a, FLINT_BITS);
    fmpz_add_ui(a, a, (ulong)a_limbs[1]);
    fmpz_mul_2exp(a, a, FLINT_BITS);
    fmpz_add_ui(a, a, (ulong)a_limbs[0]);

    fmpz_set_ui(b, 0u);
    fmpz_add_ui(b, b, (ulong)b_limbs[1]);
    fmpz_mul_2exp(b, b, FLINT_BITS);
    fmpz_add_ui(b, b, (ulong)b_limbs[0]);

    fmpz_mul(expected, a, b);
    assert(fmpz_equal(got, expected));

    fmpz_clear(got);
    fmpz_clear(expected);
    fmpz_clear(a);
    fmpz_clear(b);
}
#endif

int main(void) {
#if defined(MITHRIL_USE_FLINT)
    assert(mithril_v1_compat_init() == 0);
    test_div_fmpz_basic();
    test_rem_mod_pow_of_2_vectors();
    test_flint_kmul_equivalence();
    test_flint_kmul_limbs_equivalence();
    mithril_v1_compat_shutdown();
#endif
    puts("[OK] test_v1_compat_deferred_flint");
    return 0;
}
