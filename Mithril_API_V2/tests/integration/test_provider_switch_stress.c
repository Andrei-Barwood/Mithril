#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_bigint.h"
#include "mithril/mithril_capabilities.h"
#include "mithril/mithril_context.h"
#include "mithril/mithril_modarith.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_rng.h"

static void expect_ok(mithril_status status, const char *msg) {
    if (status != MITHRIL_OK) {
        fprintf(stderr, "[FAIL] %s: status=%d\n", msg, (int)status);
        assert(status == MITHRIL_OK);
    }
}

static void assert_active_provider(mithril_context *ctx, const char *expected) {
    const char *active = mithril_provider_active_name(ctx);
    assert(active != NULL);
    assert(strcmp(active, expected) == 0);
}

static void run_bigint_vector(mithril_context *ctx) {
    const uint8_t a[] = {0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t b[] = {0x01};
    const uint8_t expected[] = {0x01, 0x00, 0x00, 0x00, 0x00};
    uint8_t out[16] = {0};
    size_t written = 0u;
    mithril_status st;

    st = mithril_bigint_add(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    expect_ok(st, "bigint_add vector");
    assert(written == sizeof(expected));
    assert(memcmp(out, expected, sizeof(expected)) == 0);
}

static void run_modarith_vector(mithril_context *ctx) {
    const uint8_t a[] = {0x5D};
    const uint8_t b[] = {0x22};
    const uint8_t m[] = {0x65};
    const uint8_t expected[] = {0x1A};
    uint8_t out[8] = {0};
    size_t written = 0u;
    mithril_status st;

    st = mithril_modarith_add_mod(ctx, a, sizeof(a), b, sizeof(b), m, sizeof(m), out, sizeof(out), &written);
    expect_ok(st, "modarith_add_mod vector");
    assert(written == sizeof(expected));
    assert(memcmp(out, expected, sizeof(expected)) == 0);
}

static void validate_sodium_state(mithril_context *ctx) {
    mithril_capabilities caps;
    uint8_t rnd[16] = {0};
    uint8_t out[16] = {0};
    size_t written = 0u;
    mithril_status st;

    st = mithril_context_get_capabilities(ctx, &caps);
    expect_ok(st, "get capabilities sodium");
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_RNG) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_HASH) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_AEAD) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_KEX) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_SIGN) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_BIGINT) == 0);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_MODARITH) == 0);
    assert(caps.provider_count >= 3u);

    st = mithril_rng_fill(ctx, rnd, sizeof(rnd));
    expect_ok(st, "sodium rng_fill");

    st = mithril_bigint_add(ctx, rnd, 1u, rnd + 1, 1u, out, sizeof(out), &written);
    assert(st == MITHRIL_ERR_NO_PROVIDER);
}

static void validate_math_provider_state(mithril_context *ctx) {
    mithril_capabilities caps;
    uint8_t rnd[16] = {0};
    mithril_status st;

    st = mithril_context_get_capabilities(ctx, &caps);
    expect_ok(st, "get capabilities math provider");
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_BIGINT) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_MODARITH) == 1);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_RNG) == 0);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_HASH) == 0);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_AEAD) == 0);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_KEX) == 0);
    assert(mithril_capabilities_has(&caps, MITHRIL_CAP_SIGN) == 0);
    assert(caps.provider_count >= 3u);

    st = mithril_rng_fill(ctx, rnd, sizeof(rnd));
    assert(st == MITHRIL_ERR_NO_PROVIDER);

    run_bigint_vector(ctx);
    run_modarith_vector(ctx);
}

int main(void) {
    mithril_context *ctx = NULL;
    const char *cycle[] = {"sodium", "c23", "flint", "c23", "sodium", "flint"};
    size_t i;
    const size_t loops = 200u;
    mithril_status st;

    st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");
    assert(mithril_provider_count(ctx) >= 3u);
    assert_active_provider(ctx, "sodium");

    for (i = 0u; i < loops; ++i) {
        size_t j;
        for (j = 0u; j < (sizeof(cycle) / sizeof(cycle[0])); ++j) {
            st = mithril_provider_activate(ctx, cycle[j]);
            expect_ok(st, "provider_activate stress");
            assert_active_provider(ctx, cycle[j]);

            if (strcmp(cycle[j], "sodium") == 0) {
                validate_sodium_state(ctx);
            } else {
                validate_math_provider_state(ctx);
            }
        }
    }

    mithril_shutdown(ctx);
    puts("[OK] test_provider_switch_stress");
    return 0;
}
