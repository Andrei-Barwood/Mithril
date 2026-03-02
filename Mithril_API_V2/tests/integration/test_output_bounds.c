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

static void expect_status(mithril_status status, mithril_status expected, const char *msg) {
    if (status != expected) {
        fprintf(stderr, "[FAIL] %s: got=%d expected=%d\n", msg, (int)status, (int)expected);
        assert(status == expected);
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

static void test_api_argument_guards(mithril_context *ctx) {
    const uint8_t one[] = {0x01};
    uint8_t out[4] = {0};
    size_t written = 0u;
    mithril_status st;

    st = mithril_bigint_add(ctx, one, sizeof(one), one, sizeof(one), out, sizeof(out), NULL);
    expect_status(st, MITHRIL_ERR_INVALID_ARGUMENT, "bigint_add with NULL written_len");

    st = mithril_modarith_add_mod(ctx, one, sizeof(one), one, sizeof(one), one, sizeof(one), out, sizeof(out), NULL);
    expect_status(st, MITHRIL_ERR_INVALID_ARGUMENT, "modarith_add_mod with NULL written_len");
}

static void test_output_boundaries_on_math_provider(mithril_context *ctx, const char *provider_name) {
    const uint8_t add_a[] = {0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t add_b[] = {0x01};
    const uint8_t add_expected[] = {0x01, 0x00, 0x00, 0x00, 0x00};

    const uint8_t mod_a[] = {0x01, 0x00, 0x00, 0x00, 0x01};
    const uint8_t mod_b[] = {0x00, 0x00, 0x00, 0x02};
    const uint8_t mod_m[] = {0x01, 0x00, 0x00, 0x00, 0x07};
    const uint8_t mod_expected[] = {0xFF, 0xFF, 0xFF, 0xFB};

    uint8_t out[16] = {0};
    size_t written = 777u;
    mithril_status st;

    st = mithril_provider_activate(ctx, provider_name);
    expect_ok(st, "activate math provider");

    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out, 4u, &written);
    expect_status(st, MITHRIL_ERR_INVALID_ARGUMENT, "bigint_add fails on short out_len");
    assert(written == 777u);

    st = mithril_bigint_add(ctx, add_a, sizeof(add_a), add_b, sizeof(add_b), out, 5u, &written);
    expect_ok(st, "bigint_add exact out_len");
    assert(written == sizeof(add_expected));
    expect_eq_bytes(out, add_expected, sizeof(add_expected), "bigint_add exact output");

    written = 888u;
    st = mithril_modarith_mul_mod(
        ctx, mod_a, sizeof(mod_a), mod_b, sizeof(mod_b), mod_m, sizeof(mod_m), out, 3u, &written);
    expect_status(st, MITHRIL_ERR_INVALID_ARGUMENT, "modarith_mul_mod fails on short out_len");
    assert(written == 888u);

    st = mithril_modarith_mul_mod(
        ctx, mod_a, sizeof(mod_a), mod_b, sizeof(mod_b), mod_m, sizeof(mod_m), out, 4u, &written);
    expect_ok(st, "modarith_mul_mod exact out_len");
    assert(written == sizeof(mod_expected));
    expect_eq_bytes(out, mod_expected, sizeof(mod_expected), "modarith_mul_mod exact output");
}

static void test_zero_canonical_encoding(mithril_context *ctx, const char *provider_name) {
    const uint8_t a[] = {0x01};
    const uint8_t b[] = {0x01};
    const uint8_t m[] = {0x01};
    uint8_t out[8] = {0xAA};
    size_t written = 0u;
    mithril_status st;

    st = mithril_provider_activate(ctx, provider_name);
    expect_ok(st, "activate provider for canonical zero checks");

    st = mithril_bigint_sub(ctx, a, sizeof(a), b, sizeof(b), out, 1u, &written);
    expect_ok(st, "bigint_sub zero result");
    assert(written == 1u);
    assert(out[0] == 0x00u);

    out[0] = 0xAAu;
    st = mithril_modarith_mul_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, 1u, &written);
    expect_ok(st, "modarith_mul_mod zero result");
    assert(written == 1u);
    assert(out[0] == 0x00u);
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");

    test_api_argument_guards(ctx);
    test_output_boundaries_on_math_provider(ctx, "c23");
    test_output_boundaries_on_math_provider(ctx, "flint");
    test_zero_canonical_encoding(ctx, "c23");
    test_zero_canonical_encoding(ctx, "flint");

    mithril_shutdown(ctx);
    puts("[OK] test_output_bounds");
    return 0;
}
