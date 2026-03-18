#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_modarith.h"
#include "mithril/mithril_provider.h"

static void expect_ok(mithril_status status, const char *msg) {
    if (status != MITHRIL_OK) {
        fprintf(stderr, "[FAIL] %s: status=%d\n", msg, (int)status);
        assert(status == MITHRIL_OK);
    }
}

static void expect_eq_bytes(
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const char *msg) {
    if (a_len != b_len || memcmp(a, b, a_len) != 0) {
        size_t i;
        fprintf(stderr, "[FAIL] %s\n", msg);
        fprintf(stderr, "len(a)=%zu len(b)=%zu\n", a_len, b_len);
        fprintf(stderr, "a:");
        for (i = 0u; i < a_len; ++i) {
            fprintf(stderr, " %02X", a[i]);
        }
        fprintf(stderr, "\nb:");
        for (i = 0u; i < b_len; ++i) {
            fprintf(stderr, " %02X", b[i]);
        }
        fprintf(stderr, "\n");
        assert(0 && "byte mismatch");
    }
}

static void assert_active_provider(mithril_context *ctx, const char *expected) {
    const char *active = mithril_provider_active_name(ctx);
    assert(active != NULL);
    assert(strcmp(active, expected) == 0);
}

int main(void) {
    mithril_context *ctx = NULL;
    const char *active = NULL;
    uint8_t out_c23[32] = {0};
    uint8_t out_flint[32] = {0};
    size_t written_c23 = 0u;
    size_t written_flint = 0u;
    mithril_status st;

    const uint8_t add_a[] = {0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t add_b[] = {0x01};

    const uint8_t mul_a[] = {0x12, 0x34, 0x56, 0x78};
    const uint8_t mul_b[] = {0x0F, 0xED, 0xCB, 0xA9};

    const uint8_t mod_a[] = {0x01, 0x00, 0x00, 0x00, 0x01};
    const uint8_t mod_b[] = {0x00, 0x00, 0x00, 0x02};
    const uint8_t mod_m[] = {0x01, 0x00, 0x00, 0x00, 0x07};

    st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");

    assert(mithril_provider_count(ctx) >= 3u);
    active = mithril_provider_active_name(ctx);
    assert(active != NULL);
    assert(strcmp(active, "sodium") == 0);

    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out_c23, sizeof(out_c23), &written_c23);
    assert(st == MITHRIL_ERR_NO_PROVIDER);

    st = mithril_provider_activate(ctx, "c23");
    expect_ok(st, "activate c23");
    assert_active_provider(ctx, "c23");

    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "c23 bigint add");
    st = mithril_bigint_mul(ctx, mul_a, sizeof(mul_a), mul_b, sizeof(mul_b), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "c23 bigint mul");
    st = mithril_modarith_mul_mod(
        ctx, mod_a, sizeof(mod_a), mod_b, sizeof(mod_b), mod_m, sizeof(mod_m), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "c23 modarith mul_mod");

    st = mithril_provider_activate(ctx, "flint");
    expect_ok(st, "activate flint");
    assert_active_provider(ctx, "flint");

    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out_flint, sizeof(out_flint), &written_flint);
    expect_ok(st, "flint bigint add");
    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "flint bigint add repeat");
    expect_eq_bytes(out_flint, written_flint, out_c23, written_c23, "repeat flint bigint add stable");

    st = mithril_provider_activate(ctx, "c23");
    expect_ok(st, "reactivate c23");
    st = mithril_bigint_mul(ctx, mul_a, sizeof(mul_a), mul_b, sizeof(mul_b), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "c23 bigint mul compare");
    st = mithril_provider_activate(ctx, "flint");
    expect_ok(st, "reactivate flint");
    st = mithril_bigint_mul(ctx, mul_a, sizeof(mul_a), mul_b, sizeof(mul_b), out_flint, sizeof(out_flint), &written_flint);
    expect_ok(st, "flint bigint mul compare");
    expect_eq_bytes(out_c23, written_c23, out_flint, written_flint, "c23 vs flint bigint mul");

    st = mithril_provider_activate(ctx, "c23");
    expect_ok(st, "activate c23 for modarith compare");
    st = mithril_modarith_mul_mod(
        ctx, mod_a, sizeof(mod_a), mod_b, sizeof(mod_b), mod_m, sizeof(mod_m), out_c23, sizeof(out_c23), &written_c23);
    expect_ok(st, "c23 modarith compare");
    st = mithril_provider_activate(ctx, "flint");
    expect_ok(st, "activate flint for modarith compare");
    st = mithril_modarith_mul_mod(
        ctx, mod_a, sizeof(mod_a), mod_b, sizeof(mod_b), mod_m, sizeof(mod_m), out_flint, sizeof(out_flint), &written_flint);
    expect_ok(st, "flint modarith compare");
    expect_eq_bytes(out_c23, written_c23, out_flint, written_flint, "c23 vs flint modarith mul_mod");

    st = mithril_provider_activate(ctx, "sodium");
    expect_ok(st, "reactivate sodium");
    assert_active_provider(ctx, "sodium");

    mithril_shutdown(ctx);
    puts("[OK] test_provider_switch");
    return 0;
}
