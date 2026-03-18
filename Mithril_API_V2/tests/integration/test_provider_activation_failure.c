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

static void assert_active_provider(mithril_context *ctx, const char *expected) {
    const char *active = mithril_provider_active_name(ctx);
    assert(active != NULL);
    assert(strcmp(active, expected) == 0);
}

static void assert_bigint_add_still_works(mithril_context *ctx) {
    const uint8_t a[] = {0x01};
    const uint8_t b[] = {0x01};
    const uint8_t expected[] = {0x02};
    uint8_t out[8] = {0};
    size_t written = 0u;
    mithril_status st;

    st = mithril_bigint_add(ctx, a, sizeof(a), b, sizeof(b), out, sizeof(out), &written);
    expect_ok(st, "bigint add after failed activation");
    assert(written == sizeof(expected));
    assert(memcmp(out, expected, sizeof(expected)) == 0);
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st;

    st = mithril_init(&ctx, NULL);
    expect_ok(st, "mithril_init");
    assert_active_provider(ctx, "sodium");

    st = mithril_provider_activate(ctx, "c23");
    expect_ok(st, "activate c23");
    assert_active_provider(ctx, "c23");
    assert_bigint_add_still_works(ctx);

    st = mithril_provider_activate(ctx, "provider-that-does-not-exist");
    assert(st == MITHRIL_ERR_NO_PROVIDER);
    assert_active_provider(ctx, "c23");
    assert_bigint_add_still_works(ctx);

    st = mithril_provider_activate(ctx, "");
    assert(st == MITHRIL_ERR_INVALID_ARGUMENT);
    assert_active_provider(ctx, "c23");
    assert_bigint_add_still_works(ctx);

    st = mithril_provider_activate(ctx, "invalid provider name!");
    assert(st == MITHRIL_ERR_INVALID_ARGUMENT);
    assert_active_provider(ctx, "c23");
    assert_bigint_add_still_works(ctx);

    st = mithril_provider_activate(ctx, NULL);
    assert(st == MITHRIL_ERR_INVALID_ARGUMENT);
    assert_active_provider(ctx, "c23");
    assert_bigint_add_still_works(ctx);

    st = mithril_provider_activate(ctx, "sodium");
    expect_ok(st, "reactivate sodium");
    assert_active_provider(ctx, "sodium");

    mithril_shutdown(ctx);
    puts("[OK] test_provider_activation_failure");
    return 0;
}
