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
    const uint8_t *actual,
    size_t actual_len,
    const uint8_t *expected,
    size_t expected_len,
    const char *msg) {
    if (actual_len != expected_len || memcmp(actual, expected, expected_len) != 0) {
        size_t i;
        fprintf(stderr, "[FAIL] %s\n", msg);
        fprintf(stderr, "expected_len=%zu actual_len=%zu\n", expected_len, actual_len);
        fprintf(stderr, "expected:");
        for (i = 0u; i < expected_len; ++i) {
            fprintf(stderr, " %02X", expected[i]);
        }
        fprintf(stderr, "\nactual:  ");
        for (i = 0u; i < actual_len; ++i) {
            fprintf(stderr, " %02X", actual[i]);
        }
        fprintf(stderr, "\n");
        assert(0 && "bytes mismatch");
    }
}

static void activate_provider(mithril_context *ctx, const char *provider) {
    mithril_status st = mithril_provider_activate(ctx, provider);
    expect_ok(st, "provider activation");
}

static void compare_bigint_add_case(
    mithril_context *ctx,
    const char *case_name,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *expected,
    size_t expected_len) {
    uint8_t c23_out[256] = {0};
    uint8_t flint_out[256] = {0};
    size_t c23_written = 0u;
    size_t flint_written = 0u;
    mithril_status st;

    activate_provider(ctx, "c23");
    st = mithril_bigint_add(ctx, a, a_len, b, b_len, c23_out, sizeof(c23_out), &c23_written);
    expect_ok(st, case_name);

    activate_provider(ctx, "flint");
    st = mithril_bigint_add(ctx, a, a_len, b, b_len, flint_out, sizeof(flint_out), &flint_written);
    expect_ok(st, case_name);

    expect_eq_bytes(c23_out, c23_written, expected, expected_len, case_name);
    expect_eq_bytes(flint_out, flint_written, expected, expected_len, case_name);
    expect_eq_bytes(c23_out, c23_written, flint_out, flint_written, case_name);
}

static void compare_bigint_sub_case(
    mithril_context *ctx,
    const char *case_name,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *expected,
    size_t expected_len) {
    uint8_t c23_out[256] = {0};
    uint8_t flint_out[256] = {0};
    size_t c23_written = 0u;
    size_t flint_written = 0u;
    mithril_status st;

    activate_provider(ctx, "c23");
    st = mithril_bigint_sub(ctx, a, a_len, b, b_len, c23_out, sizeof(c23_out), &c23_written);
    expect_ok(st, case_name);

    activate_provider(ctx, "flint");
    st = mithril_bigint_sub(ctx, a, a_len, b, b_len, flint_out, sizeof(flint_out), &flint_written);
    expect_ok(st, case_name);

    expect_eq_bytes(c23_out, c23_written, expected, expected_len, case_name);
    expect_eq_bytes(flint_out, flint_written, expected, expected_len, case_name);
    expect_eq_bytes(c23_out, c23_written, flint_out, flint_written, case_name);
}

static void compare_bigint_mul_case(
    mithril_context *ctx,
    const char *case_name,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *expected,
    size_t expected_len) {
    uint8_t c23_out[256] = {0};
    uint8_t flint_out[256] = {0};
    size_t c23_written = 0u;
    size_t flint_written = 0u;
    mithril_status st;

    activate_provider(ctx, "c23");
    st = mithril_bigint_mul(ctx, a, a_len, b, b_len, c23_out, sizeof(c23_out), &c23_written);
    expect_ok(st, case_name);

    activate_provider(ctx, "flint");
    st = mithril_bigint_mul(ctx, a, a_len, b, b_len, flint_out, sizeof(flint_out), &flint_written);
    expect_ok(st, case_name);

    expect_eq_bytes(c23_out, c23_written, expected, expected_len, case_name);
    expect_eq_bytes(flint_out, flint_written, expected, expected_len, case_name);
    expect_eq_bytes(c23_out, c23_written, flint_out, flint_written, case_name);
}

static void compare_modarith_add_mod_case(
    mithril_context *ctx,
    const char *case_name,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    const uint8_t *expected,
    size_t expected_len) {
    uint8_t c23_out[256] = {0};
    uint8_t flint_out[256] = {0};
    size_t c23_written = 0u;
    size_t flint_written = 0u;
    mithril_status st;

    activate_provider(ctx, "c23");
    st = mithril_modarith_add_mod(
        ctx, a, a_len, b, b_len, modulus, modulus_len, c23_out, sizeof(c23_out), &c23_written);
    expect_ok(st, case_name);

    activate_provider(ctx, "flint");
    st = mithril_modarith_add_mod(
        ctx, a, a_len, b, b_len, modulus, modulus_len, flint_out, sizeof(flint_out), &flint_written);
    expect_ok(st, case_name);

    expect_eq_bytes(c23_out, c23_written, expected, expected_len, case_name);
    expect_eq_bytes(flint_out, flint_written, expected, expected_len, case_name);
    expect_eq_bytes(c23_out, c23_written, flint_out, flint_written, case_name);
}

static void compare_modarith_mul_mod_case(
    mithril_context *ctx,
    const char *case_name,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    const uint8_t *modulus,
    size_t modulus_len,
    const uint8_t *expected,
    size_t expected_len) {
    uint8_t c23_out[256] = {0};
    uint8_t flint_out[256] = {0};
    size_t c23_written = 0u;
    size_t flint_written = 0u;
    mithril_status st;

    activate_provider(ctx, "c23");
    st = mithril_modarith_mul_mod(
        ctx, a, a_len, b, b_len, modulus, modulus_len, c23_out, sizeof(c23_out), &c23_written);
    expect_ok(st, case_name);

    activate_provider(ctx, "flint");
    st = mithril_modarith_mul_mod(
        ctx, a, a_len, b, b_len, modulus, modulus_len, flint_out, sizeof(flint_out), &flint_written);
    expect_ok(st, case_name);

    expect_eq_bytes(c23_out, c23_written, expected, expected_len, case_name);
    expect_eq_bytes(flint_out, flint_written, expected, expected_len, case_name);
    expect_eq_bytes(c23_out, c23_written, flint_out, flint_written, case_name);
}

static void run_bigint_add_vectors(mithril_context *ctx) {
    {
        const uint8_t a[] = {0x00};
        const uint8_t b[] = {0x00};
        const uint8_t expected[] = {0x00};
        compare_bigint_add_case(ctx, "add_zero_zero", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0xFF};
        const uint8_t b[] = {0x01};
        const uint8_t expected[] = {0x01, 0x00};
        compare_bigint_add_case(ctx, "add_carry_8bit", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x00, 0xFF, 0xFF};
        const uint8_t b[] = {0x00, 0x01};
        const uint8_t expected[] = {0x01, 0x00, 0x00};
        compare_bigint_add_case(ctx, "add_leading_zeros", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x12, 0x34, 0x56, 0x78};
        const uint8_t b[] = {0x0F, 0xED, 0xCB, 0xA9};
        const uint8_t expected[] = {0x22, 0x22, 0x22, 0x21};
        compare_bigint_add_case(ctx, "add_pattern", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0xFF, 0xFF, 0xFF, 0xFF};
        const uint8_t b[] = {0x01};
        const uint8_t expected[] = {0x01, 0x00, 0x00, 0x00, 0x00};
        compare_bigint_add_case(ctx, "add_32bit_wrap", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
}

static void run_bigint_sub_vectors(mithril_context *ctx) {
    {
        const uint8_t a[] = {0x01};
        const uint8_t b[] = {0x01};
        const uint8_t expected[] = {0x00};
        compare_bigint_sub_case(ctx, "sub_equal", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x01, 0x00};
        const uint8_t b[] = {0x01};
        const uint8_t expected[] = {0xFF};
        compare_bigint_sub_case(ctx, "sub_borrow", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x00, 0x12, 0x34, 0x56};
        const uint8_t b[] = {0x00, 0x00, 0x34, 0x56};
        const uint8_t expected[] = {0x12, 0x00, 0x00};
        compare_bigint_sub_case(ctx, "sub_leading_zeros", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x22, 0x22, 0x22, 0x21};
        const uint8_t b[] = {0x0F, 0xED, 0xCB, 0xA9};
        const uint8_t expected[] = {0x12, 0x34, 0x56, 0x78};
        compare_bigint_sub_case(ctx, "sub_pattern", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x01, 0x00, 0x00, 0x00, 0x00};
        const uint8_t b[] = {0x01};
        const uint8_t expected[] = {0xFF, 0xFF, 0xFF, 0xFF};
        compare_bigint_sub_case(ctx, "sub_large", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
}

static void run_bigint_mul_vectors(mithril_context *ctx) {
    {
        const uint8_t a[] = {0x00};
        const uint8_t b[] = {0xAB};
        const uint8_t expected[] = {0x00};
        compare_bigint_mul_case(ctx, "mul_zero", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0xFF};
        const uint8_t b[] = {0xFF};
        const uint8_t expected[] = {0xFE, 0x01};
        compare_bigint_mul_case(ctx, "mul_8bit", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x00, 0x12, 0x34};
        const uint8_t b[] = {0x00, 0x02};
        const uint8_t expected[] = {0x24, 0x68};
        compare_bigint_mul_case(ctx, "mul_leading_zeros", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x12, 0x34};
        const uint8_t b[] = {0x56, 0x78};
        const uint8_t expected[] = {0x06, 0x26, 0x00, 0x60};
        compare_bigint_mul_case(ctx, "mul_pattern", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        const uint8_t b[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        const uint8_t expected[] = {
            0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
        };
        compare_bigint_mul_case(ctx, "mul_64x64", a, sizeof(a), b, sizeof(b), expected, sizeof(expected));
    }
}

static void run_modarith_add_vectors(mithril_context *ctx) {
    {
        const uint8_t a[] = {0x5D};
        const uint8_t b[] = {0x22};
        const uint8_t m[] = {0x65};
        const uint8_t expected[] = {0x1A};
        compare_modarith_add_mod_case(ctx, "add_mod_small", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x01, 0x00, 0x00, 0x00, 0x06};
        const uint8_t b[] = {0x00, 0x00, 0x00, 0x05};
        const uint8_t m[] = {0x01, 0x00, 0x00, 0x00, 0x07};
        const uint8_t expected[] = {0x04};
        compare_modarith_add_mod_case(ctx, "add_mod_wrap", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x12, 0x34};
        const uint8_t b[] = {0x56, 0x78};
        const uint8_t m[] = {0x01};
        const uint8_t expected[] = {0x00};
        compare_modarith_add_mod_case(ctx, "add_mod_one", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x00, 0x64};
        const uint8_t b[] = {0x00, 0x37};
        const uint8_t m[] = {0x00, 0x65};
        const uint8_t expected[] = {0x36};
        compare_modarith_add_mod_case(ctx, "add_mod_leading_zeros", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0xAA, 0xBB, 0xCC};
        const uint8_t b[] = {0x11, 0x22, 0x33};
        const uint8_t m[] = {0xFF, 0xFF, 0xFE};
        const uint8_t expected[] = {0xBB, 0xDD, 0xFF};
        compare_modarith_add_mod_case(ctx, "add_mod_no_wrap", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
}

static void run_modarith_mul_vectors(mithril_context *ctx) {
    {
        const uint8_t a[] = {0x12};
        const uint8_t b[] = {0x0E};
        const uint8_t m[] = {0x17};
        const uint8_t expected[] = {0x16};
        compare_modarith_mul_mod_case(ctx, "mul_mod_small", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x01,0x00,0x00,0x00,0x01};
        const uint8_t b[] = {0x00,0x00,0x00,0x02};
        const uint8_t m[] = {0x01,0x00,0x00,0x00,0x07};
        const uint8_t expected[] = {0xFF,0xFF,0xFF,0xFB};
        compare_modarith_mul_mod_case(ctx, "mul_mod_large", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x12, 0x34};
        const uint8_t b[] = {0x56, 0x78};
        const uint8_t m[] = {0x01};
        const uint8_t expected[] = {0x00};
        compare_modarith_mul_mod_case(ctx, "mul_mod_one", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x00, 0x80};
        const uint8_t b[] = {0x00, 0x02};
        const uint8_t m[] = {0x00, 0x7F};
        const uint8_t expected[] = {0x02};
        compare_modarith_mul_mod_case(ctx, "mul_mod_leading_zeros", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
    {
        const uint8_t a[] = {0x0A, 0x0B};
        const uint8_t b[] = {0x00, 0x00, 0x02};
        const uint8_t m[] = {0x10, 0x00};
        const uint8_t expected[] = {0x04, 0x16};
        compare_modarith_mul_mod_case(ctx, "mul_mod_simple", a, sizeof(a), b, sizeof(b), m, sizeof(m), expected, sizeof(expected));
    }
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");

    run_bigint_add_vectors(ctx);
    run_bigint_sub_vectors(ctx);
    run_bigint_mul_vectors(ctx);
    run_modarith_add_vectors(ctx);
    run_modarith_mul_vectors(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_conformance_vectors");
    return 0;
}
