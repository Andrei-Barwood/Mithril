#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_aead.h"
#include "mithril/mithril_api.h"
#include "mithril/mithril_capabilities.h"
#include "mithril/mithril_error.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_rng.h"
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

static mithril_status fake_rng_unknown_status(void *user_data, uint8_t *out, size_t out_len) {
    (void)user_data;
    if (out == NULL || out_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
    out[0] = 0u;
    return (mithril_status)77;
}

static mithril_status fake_rng_ok(void *user_data, uint8_t *out, size_t out_len) {
    (void)user_data;
    if (out == NULL || out_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }
    memset(out, 0xA5, out_len);
    return MITHRIL_OK;
}

static mithril_status fake_on_activate_returns_sig_invalid(void *user_data) {
    (void)user_data;
    return MITHRIL_ERR_SIGNATURE_INVALID;
}

static mithril_status fake_on_activate_returns_unknown(void *user_data) {
    (void)user_data;
    return (mithril_status)88;
}

static mithril_status fake_get_capabilities_ok(void *user_data, mithril_capabilities *out_caps) {
    (void)user_data;
    if (out_caps == NULL) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    out_caps->feature_flags = MITHRIL_CAP_RNG;
    out_caps->provider_count = 1u;
    out_caps->reserved = 0u;
    return MITHRIL_OK;
}

static mithril_status fake_get_capabilities_unknown(void *user_data, mithril_capabilities *out_caps) {
    (void)user_data;
    (void)out_caps;
    return (mithril_status)99;
}

static void test_status_string_contract(void) {
    assert(strcmp(mithril_status_string(MITHRIL_ERR_AEAD_AUTH_FAILED), "MITHRIL_ERR_AEAD_AUTH_FAILED") == 0);
    assert(strcmp(mithril_status_string(MITHRIL_ERR_SIGNATURE_INVALID), "MITHRIL_ERR_SIGNATURE_INVALID") == 0);
}

static void test_verification_error_contracts(mithril_context *ctx) {
    const uint32_t aead_alg = MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF;
    const uint32_t sign_alg = MITHRIL_SIGN_ALG_ED25519;

    const uint8_t aad[] = {'a', 'a', 'd'};
    const uint8_t plaintext[] = {'m', 'i', 't', 'h', 'r', 'i', 'l'};
    const uint8_t message[] = {'s', 'i', 'g', 'n', '-', 'm', 's', 'g'};

    uint8_t key[32] = {0};
    uint8_t nonce[24] = {0};
    uint8_t ciphertext[128] = {0};
    uint8_t tampered_ciphertext[128] = {0};
    uint8_t plaintext_out[128] = {0};

    uint8_t pk[32] = {0};
    uint8_t sk[64] = {0};
    uint8_t sig[64] = {0};
    uint8_t tampered_sig[64] = {0};

    size_t ct_written = 0u;
    size_t pt_written = 0u;
    size_t sig_written = 0u;
    mithril_status st;

    expect_ok(mithril_provider_activate(ctx, "sodium"), "activate sodium for verification contracts");

    expect_ok(mithril_rng_fill(ctx, key, sizeof(key)), "rng key");
    expect_ok(mithril_rng_fill(ctx, nonce, sizeof(nonce)), "rng nonce");

    st = mithril_aead_encrypt(
        ctx,
        aead_alg,
        key,
        sizeof(key),
        nonce,
        sizeof(nonce),
        aad,
        sizeof(aad),
        plaintext,
        sizeof(plaintext),
        ciphertext,
        sizeof(ciphertext),
        &ct_written);
    expect_ok(st, "aead_encrypt baseline");

    memcpy(tampered_ciphertext, ciphertext, ct_written);
    tampered_ciphertext[0] ^= 0x01u;

    st = mithril_aead_decrypt(
        ctx,
        aead_alg,
        key,
        sizeof(key),
        nonce,
        sizeof(nonce),
        aad,
        sizeof(aad),
        tampered_ciphertext,
        ct_written,
        plaintext_out,
        sizeof(plaintext_out),
        &pt_written);
    expect_status(st, MITHRIL_ERR_AEAD_AUTH_FAILED, "tampered AEAD decrypt returns explicit auth failure");

    expect_ok(mithril_sign_keypair(ctx, sign_alg, pk, sizeof(pk), sk, sizeof(sk)), "sign keypair baseline");
    expect_ok(
        mithril_sign_detached(
            ctx,
            sign_alg,
            message,
            sizeof(message),
            sk,
            sizeof(sk),
            sig,
            sizeof(sig),
            &sig_written),
        "sign_detached baseline");

    memcpy(tampered_sig, sig, sig_written);
    tampered_sig[0] ^= 0x80u;

    st = mithril_sign_verify_detached(
        ctx,
        sign_alg,
        message,
        sizeof(message),
        pk,
        sizeof(pk),
        tampered_sig,
        sig_written);
    expect_status(st, MITHRIL_ERR_SIGNATURE_INVALID, "tampered signature returns explicit verification failure");
}

static void test_no_provider_contracts(mithril_context *ctx) {
    uint8_t out[64] = {0};
    size_t written = 0u;
    static const uint8_t one[] = {0x01};

    expect_ok(mithril_provider_activate(ctx, "c23"), "activate c23 for no-provider checks");

    expect_status(
        mithril_aead_encrypt(
            ctx,
            MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF,
            one,
            sizeof(one),
            one,
            sizeof(one),
            one,
            sizeof(one),
            one,
            sizeof(one),
            out,
            sizeof(out),
            &written),
        MITHRIL_ERR_NO_PROVIDER,
        "aead on c23 returns no-provider");

    expect_status(
        mithril_sign_verify_detached(
            ctx,
            MITHRIL_SIGN_ALG_ED25519,
            one,
            sizeof(one),
            one,
            sizeof(one),
            one,
            sizeof(one)),
        MITHRIL_ERR_NO_PROVIDER,
        "sign verify on c23 returns no-provider");
}

static void test_normalized_crypto_status_mapping(mithril_context *ctx) {
    static const mithril_provider_crypto_ops fake_crypto_ops = {
        .rng_fill = fake_rng_unknown_status
    };

    const mithril_provider_descriptor descriptor = {
        .name = "errmap_unknown_rng",
        .abi_version = MITHRIL_PROVIDER_ABI_VERSION,
        .declared_capabilities = MITHRIL_CAP_RNG,
        .ops = NULL,
        .crypto_ops = &fake_crypto_ops,
        .user_data = NULL
    };

    uint8_t out[8] = {0};
    mithril_status st;

    st = mithril_provider_register(ctx, &descriptor);
    expect_ok(st, "register unknown-status rng provider");

    st = mithril_provider_activate(ctx, descriptor.name);
    expect_ok(st, "activate unknown-status rng provider");

    st = mithril_rng_fill(ctx, out, sizeof(out));
    expect_status(st, MITHRIL_ERR_INTERNAL, "unknown provider crypto status maps to internal");
}

static void test_normalized_lifecycle_status_mapping(mithril_context *ctx) {
    static const mithril_provider_crypto_ops fake_crypto_ops = {
        .rng_fill = fake_rng_ok
    };

    static const mithril_provider_ops ops_sig_invalid = {
        .on_activate = fake_on_activate_returns_sig_invalid,
        .on_deactivate = NULL,
        .get_capabilities = fake_get_capabilities_ok
    };

    static const mithril_provider_ops ops_unknown_activate = {
        .on_activate = fake_on_activate_returns_unknown,
        .on_deactivate = NULL,
        .get_capabilities = fake_get_capabilities_ok
    };

    static const mithril_provider_ops ops_unknown_caps = {
        .on_activate = NULL,
        .on_deactivate = NULL,
        .get_capabilities = fake_get_capabilities_unknown
    };

    const mithril_provider_descriptor d_sig_invalid = {
        .name = "errmap_activate_siginv",
        .abi_version = MITHRIL_PROVIDER_ABI_VERSION,
        .declared_capabilities = MITHRIL_CAP_RNG,
        .ops = &ops_sig_invalid,
        .crypto_ops = &fake_crypto_ops,
        .user_data = NULL
    };

    const mithril_provider_descriptor d_unknown_activate = {
        .name = "errmap_activate_unknown",
        .abi_version = MITHRIL_PROVIDER_ABI_VERSION,
        .declared_capabilities = MITHRIL_CAP_RNG,
        .ops = &ops_unknown_activate,
        .crypto_ops = &fake_crypto_ops,
        .user_data = NULL
    };

    const mithril_provider_descriptor d_unknown_caps = {
        .name = "errmap_caps_unknown",
        .abi_version = MITHRIL_PROVIDER_ABI_VERSION,
        .declared_capabilities = MITHRIL_CAP_RNG,
        .ops = &ops_unknown_caps,
        .crypto_ops = &fake_crypto_ops,
        .user_data = NULL
    };

    mithril_status st;

    st = mithril_provider_register(ctx, &d_sig_invalid);
    expect_ok(st, "register lifecycle(sig invalid) provider");
    st = mithril_provider_activate(ctx, d_sig_invalid.name);
    expect_status(st, MITHRIL_ERR_INTERNAL, "activation maps verification-like lifecycle status to internal");

    st = mithril_provider_register(ctx, &d_unknown_activate);
    expect_ok(st, "register lifecycle(unknown) provider");
    st = mithril_provider_activate(ctx, d_unknown_activate.name);
    expect_status(st, MITHRIL_ERR_INTERNAL, "activation maps unknown lifecycle status to internal");

    st = mithril_provider_register(ctx, &d_unknown_caps);
    expect_status(st, MITHRIL_ERR_INTERNAL, "register maps unknown get_capabilities status to internal");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);

    if (st == MITHRIL_ERR_NOT_IMPLEMENTED) {
        puts("[SKIP] test_error_contracts (sodium provider not enabled at build time)");
        return 0;
    }

    expect_ok(st, "mithril_init");

    test_status_string_contract();
    test_verification_error_contracts(ctx);
    test_no_provider_contracts(ctx);
    test_normalized_crypto_status_mapping(ctx);
    test_normalized_lifecycle_status_mapping(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_error_contracts");
    return 0;
}
