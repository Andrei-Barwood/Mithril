#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_hash.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_rng.h"

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

static void expect_eq_bytes(const uint8_t *actual, const uint8_t *expected, size_t len, const char *msg) {
    if (memcmp(actual, expected, len) != 0) {
        size_t i;
        fprintf(stderr, "[FAIL] %s\nexpected:", msg);
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", expected[i]);
        }
        fprintf(stderr, "\nactual:  ");
        for (i = 0u; i < len; ++i) {
            fprintf(stderr, " %02X", actual[i]);
        }
        fprintf(stderr, "\n");
        assert(0 && "bytes mismatch");
    }
}

static void activate_sodium(mithril_context *ctx) {
    mithril_status st = mithril_provider_activate(ctx, "sodium");
    expect_ok(st, "activate sodium provider");
}

static void test_rng_guards(mithril_context *ctx) {
    uint8_t out[32] = {0};
    uint64_t value = 0u;

    expect_status(
        mithril_rng_fill(ctx, NULL, sizeof(out)),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "rng_fill rejects NULL out");

    expect_status(
        mithril_rng_fill(ctx, out, 0u),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "rng_fill rejects zero length");

    expect_status(
        mithril_rng_u64(ctx, NULL),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "rng_u64 rejects NULL out_value");

    expect_ok(mithril_rng_fill(ctx, out, sizeof(out)), "rng_fill success");
    expect_ok(mithril_rng_u64(ctx, &value), "rng_u64 success");
}

static void test_hash_sizes(mithril_context *ctx) {
    assert(mithril_hash_digest_size(ctx, MITHRIL_HASH_ALG_BLAKE2B_256) == 32u);
    assert(mithril_hash_digest_size(ctx, 999u) == 0u);
    assert(mithril_hash_digest_size(NULL, MITHRIL_HASH_ALG_BLAKE2B_256) == 0u);
}

static void test_hash_known_vectors(mithril_context *ctx) {
    static const uint8_t expected_abc[32] = {
        0xBD, 0xDD, 0x81, 0x3C, 0x63, 0x42, 0x39, 0x72,
        0x31, 0x71, 0xEF, 0x3F, 0xEE, 0x98, 0x57, 0x9B,
        0x94, 0x96, 0x4E, 0x3B, 0xB1, 0xCB, 0x3E, 0x42,
        0x72, 0x62, 0xC8, 0xC0, 0x68, 0xD5, 0x23, 0x19
    };
    static const uint8_t expected_empty[32] = {
        0x0E, 0x57, 0x51, 0xC0, 0x26, 0xE5, 0x43, 0xB2,
        0xE8, 0xAB, 0x2E, 0xB0, 0x60, 0x99, 0xDA, 0xA1,
        0xD1, 0xE5, 0xDF, 0x47, 0x77, 0x8F, 0x77, 0x87,
        0xFA, 0xAB, 0x45, 0xCD, 0xF1, 0x2F, 0xE3, 0xA8
    };

    const uint8_t input_abc[] = {'a', 'b', 'c'};
    const uint8_t dummy_input[] = {0x00};
    uint8_t out[64] = {0};
    size_t written = 0u;

    expect_ok(
        mithril_hash_compute(
            ctx,
            MITHRIL_HASH_ALG_BLAKE2B_256,
            input_abc,
            sizeof(input_abc),
            out,
            sizeof(out),
            &written),
        "hash(abc) succeeds");
    assert(written == 32u);
    expect_eq_bytes(out, expected_abc, 32u, "hash(abc) vector");

    expect_ok(
        mithril_hash_compute(
            ctx,
            MITHRIL_HASH_ALG_BLAKE2B_256,
            dummy_input,
            0u,
            out,
            sizeof(out),
            &written),
        "hash(empty) succeeds");
    assert(written == 32u);
    expect_eq_bytes(out, expected_empty, 32u, "hash(empty) vector");
}

static void test_hash_guards(mithril_context *ctx) {
    const uint8_t input[] = {0xAA, 0xBB, 0xCC};
    uint8_t out[32] = {0};
    size_t written = 123u;

    expect_status(
        mithril_hash_compute(
            ctx,
            MITHRIL_HASH_ALG_BLAKE2B_256,
            NULL,
            sizeof(input),
            out,
            sizeof(out),
            &written),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "hash rejects NULL input");

    expect_status(
        mithril_hash_compute(
            ctx,
            MITHRIL_HASH_ALG_BLAKE2B_256,
            input,
            sizeof(input),
            out,
            sizeof(out) - 1u,
            &written),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "hash rejects short output buffer");

    expect_status(
        mithril_hash_compute(
            ctx,
            999u,
            input,
            sizeof(input),
            out,
            sizeof(out),
            &written),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "hash rejects unknown algorithm");

    expect_status(
        mithril_hash_compute(
            ctx,
            MITHRIL_HASH_ALG_BLAKE2B_256,
            input,
            sizeof(input),
            out,
            sizeof(out),
            NULL),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "hash rejects NULL written_len");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);

    if (st == MITHRIL_ERR_NOT_IMPLEMENTED) {
        puts("[SKIP] test_sodium_rng_hash (sodium provider not enabled at build time)");
        return 0;
    }

    expect_ok(st, "mithril_init");
    activate_sodium(ctx);

    test_rng_guards(ctx);
    test_hash_sizes(ctx);
    test_hash_known_vectors(ctx);
    test_hash_guards(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_sodium_rng_hash");
    return 0;
}
