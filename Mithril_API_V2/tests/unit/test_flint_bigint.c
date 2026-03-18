#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_provider.h"

static void expect_ok(mithril_status status, const char *msg) {
    if (status != MITHRIL_OK) {
        fprintf(stderr, "[FAIL] %s: status=%d\n", msg, (int)status);
        assert(status == MITHRIL_OK);
    }
}

static void expect_eq_bytes(const uint8_t *a, const uint8_t *b, size_t len, const char *msg) {
    if (memcmp(a, b, len) != 0) {
        size_t i;
        fprintf(stderr, "[FAIL] %s\nexpected:", msg);
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", b[i]);
        }
        fprintf(stderr, "\nactual:  ");
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", a[i]);
        }
        fprintf(stderr, "\n");
        assert(0 && "bytes mismatch");
    }
}

static void switch_to_flint(mithril_context *ctx) {
    mithril_status st = mithril_provider_activate(ctx, "flint");
    expect_ok(st, "activate flint provider");
}

static void test_add_large_carry(mithril_context *ctx) {
    const uint8_t a[] = {0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t b[] = {0x01};
    const uint8_t expected[] = {0x01, 0x00, 0x00, 0x00, 0x00};
    uint8_t out[16] = {0};
    size_t written = 0u;

    mithril_status st = mithril_bigint_add(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    expect_ok(st, "bigint add large carry");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "bigint add output");
}

static void test_sub_equal_values(mithril_context *ctx) {
    const uint8_t a[] = {0x12, 0x34, 0x56};
    const uint8_t b[] = {0x12, 0x34, 0x56};
    const uint8_t expected[] = {0x00};
    uint8_t out[16] = {0};
    size_t written = 0u;

    mithril_status st = mithril_bigint_sub(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    expect_ok(st, "bigint sub equal values");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "bigint sub zero output");
}

static void test_sub_negative_rejected(mithril_context *ctx) {
    const uint8_t a[] = {0x01};
    const uint8_t b[] = {0x02};
    uint8_t out[4] = {0};
    size_t written = 0u;

    mithril_status st = mithril_bigint_sub(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    assert(st == MITHRIL_ERR_INVALID_ARGUMENT);
}

static void test_mul_64x64_to_128(mithril_context *ctx) {
    const uint8_t a[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    const uint8_t b[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    const uint8_t expected[] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };
    uint8_t out[32] = {0};
    size_t written = 0u;

    mithril_status st = mithril_bigint_mul(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    expect_ok(st, "bigint mul 64x64");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "bigint mul output");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");

    switch_to_flint(ctx);

    test_add_large_carry(ctx);
    test_sub_equal_values(ctx);
    test_sub_negative_rejected(ctx);
    test_mul_64x64_to_128(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_flint_bigint");
    return 0;
}
