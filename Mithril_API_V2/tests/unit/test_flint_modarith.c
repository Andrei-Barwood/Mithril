#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_modarith.h"
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

static void test_add_mod_small(mithril_context *ctx) {
    const uint8_t a[] = {0x5D};
    const uint8_t b[] = {0x22};
    const uint8_t m[] = {0x65};
    const uint8_t expected[] = {0x1A};
    uint8_t out[8] = {0};
    size_t written = 0u;

    mithril_status st = mithril_modarith_add_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, sizeof(out), &written);
    expect_ok(st, "modarith add_mod small");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "modarith add_mod output");
}

static void test_mul_mod_small(mithril_context *ctx) {
    const uint8_t a[] = {0x12};
    const uint8_t b[] = {0x0E};
    const uint8_t m[] = {0x17};
    const uint8_t expected[] = {0x16};
    uint8_t out[8] = {0};
    size_t written = 0u;

    mithril_status st = mithril_modarith_mul_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, sizeof(out), &written);
    expect_ok(st, "modarith mul_mod small");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "modarith mul_mod output");
}

static void test_mod_zero_rejected(mithril_context *ctx) {
    const uint8_t a[] = {0x01};
    const uint8_t b[] = {0x01};
    const uint8_t m[] = {0x00};
    uint8_t out[8] = {0};
    size_t written = 0u;

    mithril_status st = mithril_modarith_add_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, sizeof(out), &written);
    assert(st == MITHRIL_ERR_INVALID_ARGUMENT);
}

static void test_mul_mod_large(mithril_context *ctx) {
    const uint8_t a[] = {0x01,0x00,0x00,0x00,0x01};
    const uint8_t b[] = {0x00,0x00,0x00,0x02};
    const uint8_t m[] = {0x01,0x00,0x00,0x00,0x07};
    const uint8_t expected[] = {0xFF,0xFF,0xFF,0xFB};
    uint8_t out[16] = {0};
    size_t written = 0u;

    mithril_status st = mithril_modarith_mul_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, sizeof(out), &written);
    expect_ok(st, "modarith mul_mod large");
    assert(written == sizeof(expected));
    expect_eq_bytes(out, expected, sizeof(expected), "modarith mul_mod large output");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");

    switch_to_flint(ctx);

    test_add_mod_small(ctx);
    test_mul_mod_small(ctx);
    test_mod_zero_rejected(ctx);
    test_mul_mod_large(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_flint_modarith");
    return 0;
}
