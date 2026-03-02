#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_sign.h"

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

static void activate_sodium(mithril_context *ctx) {
    mithril_status st = mithril_provider_activate(ctx, "sodium");
    expect_ok(st, "activate sodium provider");
}

static void test_sign_sizes(mithril_context *ctx) {
    assert(mithril_sign_public_key_size(ctx, MITHRIL_SIGN_ALG_ED25519) == 32u);
    assert(mithril_sign_secret_key_size(ctx, MITHRIL_SIGN_ALG_ED25519) == 64u);
    assert(mithril_sign_signature_size(ctx, MITHRIL_SIGN_ALG_ED25519) == 64u);

    assert(mithril_sign_public_key_size(ctx, 999u) == 0u);
    assert(mithril_sign_secret_key_size(ctx, 999u) == 0u);
    assert(mithril_sign_signature_size(ctx, 999u) == 0u);
}

static void test_sign_verify_negative_cases(mithril_context *ctx) {
    const uint32_t alg = MITHRIL_SIGN_ALG_ED25519;
    const uint8_t message[] = {
        'M', 'i', 't', 'h', 'r', 'i', 'l', ' ', 'S', 'p', 'r', 'i', 'n', 't', ' ', '3', ' ', 'S', 'I', 'G', 'N'
    };

    uint8_t public_key[32] = {0};
    uint8_t secret_key[64] = {0};
    uint8_t other_public_key[32] = {0};
    uint8_t other_secret_key[64] = {0};
    uint8_t signature[64] = {0};
    uint8_t tampered_signature[64] = {0};
    uint8_t tampered_message[sizeof(message)] = {0};

    size_t pk_len = mithril_sign_public_key_size(ctx, alg);
    size_t sk_len = mithril_sign_secret_key_size(ctx, alg);
    size_t sig_len = mithril_sign_signature_size(ctx, alg);
    size_t written_len = 0u;

    assert(pk_len == sizeof(public_key));
    assert(sk_len == sizeof(secret_key));
    assert(sig_len == sizeof(signature));

    expect_ok(
        mithril_sign_keypair(ctx, alg, public_key, pk_len, secret_key, sk_len),
        "sign keypair primary");

    expect_ok(
        mithril_sign_keypair(ctx, alg, other_public_key, pk_len, other_secret_key, sk_len),
        "sign keypair secondary");

    expect_ok(
        mithril_sign_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            secret_key,
            sk_len,
            signature,
            sizeof(signature),
            &written_len),
        "sign_detached valid");
    assert(written_len == sig_len);

    expect_ok(
        mithril_sign_verify_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            public_key,
            pk_len,
            signature,
            written_len),
        "sign_verify valid");

    memcpy(tampered_message, message, sizeof(message));
    tampered_message[0] ^= 0x01u;

    expect_status(
        mithril_sign_verify_detached(
            ctx,
            alg,
            tampered_message,
            sizeof(tampered_message),
            public_key,
            pk_len,
            signature,
            written_len),
        MITHRIL_ERR_INTERNAL,
        "sign_verify rejects tampered message");

    memcpy(tampered_signature, signature, written_len);
    tampered_signature[0] ^= 0x80u;

    expect_status(
        mithril_sign_verify_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            public_key,
            pk_len,
            tampered_signature,
            written_len),
        MITHRIL_ERR_INTERNAL,
        "sign_verify rejects tampered signature");

    expect_status(
        mithril_sign_verify_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            other_public_key,
            pk_len,
            signature,
            written_len),
        MITHRIL_ERR_INTERNAL,
        "sign_verify rejects wrong public key");

    expect_status(
        mithril_sign_verify_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            public_key,
            pk_len,
            signature,
            sig_len - 1u),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "sign_verify rejects short signature length");

    expect_status(
        mithril_sign_detached(
            ctx,
            alg,
            message,
            sizeof(message),
            secret_key,
            sk_len - 1u,
            signature,
            sizeof(signature),
            &written_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "sign_detached rejects short secret key");

    expect_status(
        mithril_sign_detached(
            ctx,
            999u,
            message,
            sizeof(message),
            secret_key,
            sk_len,
            signature,
            sizeof(signature),
            &written_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "sign_detached rejects unsupported algorithm");

    expect_status(
        mithril_sign_keypair(ctx, 999u, public_key, pk_len, secret_key, sk_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "sign_keypair rejects unsupported algorithm");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);

    if (st == MITHRIL_ERR_NOT_IMPLEMENTED) {
        puts("[SKIP] test_sodium_sign (sodium provider not enabled at build time)");
        return 0;
    }

    expect_ok(st, "mithril_init");
    activate_sodium(ctx);

    test_sign_sizes(ctx);
    test_sign_verify_negative_cases(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_sodium_sign");
    return 0;
}
