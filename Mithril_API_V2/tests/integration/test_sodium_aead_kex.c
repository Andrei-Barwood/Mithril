#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mithril/mithril_aead.h"
#include "mithril/mithril_api.h"
#include "mithril/mithril_kex.h"
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

static void activate_sodium(mithril_context *ctx) {
    mithril_status st = mithril_provider_activate(ctx, "sodium");
    expect_ok(st, "activate sodium provider");
}

static void test_aead_roundtrip(mithril_context *ctx) {
    const uint8_t aad[] = {'M', 'i', 't', 'h', 'r', 'i', 'l', '-', 'A', 'A', 'D'};
    const uint8_t plaintext[] = {
        'S', 'p', 'r', 'i', 'n', 't', '3', ':', ' ', 'A', 'E', 'A', 'D', ' ',
        'r', 'o', 'u', 'n', 'd', 't', 'r', 'i', 'p'
    };
    const uint32_t alg = MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF;

    uint8_t key[32] = {0};
    uint8_t nonce[24] = {0};
    uint8_t ciphertext[128] = {0};
    uint8_t plaintext_out[128] = {0};
    uint8_t tampered[128] = {0};

    size_t key_len = mithril_aead_key_size(ctx, alg);
    size_t nonce_len = mithril_aead_nonce_size(ctx, alg);
    size_t tag_len = mithril_aead_tag_size(ctx, alg);
    size_t ct_written = 0u;
    size_t pt_written = 0u;
    mithril_status st;

    assert(key_len == sizeof(key));
    assert(nonce_len == sizeof(nonce));
    assert(tag_len == 16u);

    expect_ok(mithril_rng_fill(ctx, key, key_len), "rng_fill aead key");
    expect_ok(mithril_rng_fill(ctx, nonce, nonce_len), "rng_fill aead nonce");

    st = mithril_aead_encrypt(
        ctx,
        alg,
        key,
        key_len,
        nonce,
        nonce_len,
        aad,
        sizeof(aad),
        plaintext,
        sizeof(plaintext),
        ciphertext,
        sizeof(ciphertext),
        &ct_written);
    expect_ok(st, "aead_encrypt roundtrip");
    assert(ct_written == sizeof(plaintext) + tag_len);

    st = mithril_aead_decrypt(
        ctx,
        alg,
        key,
        key_len,
        nonce,
        nonce_len,
        aad,
        sizeof(aad),
        ciphertext,
        ct_written,
        plaintext_out,
        sizeof(plaintext_out),
        &pt_written);
    expect_ok(st, "aead_decrypt roundtrip");
    assert(pt_written == sizeof(plaintext));
    assert(memcmp(plaintext_out, plaintext, sizeof(plaintext)) == 0);

    memcpy(tampered, ciphertext, ct_written);
    tampered[0] ^= 0x01u;

    st = mithril_aead_decrypt(
        ctx,
        alg,
        key,
        key_len,
        nonce,
        nonce_len,
        aad,
        sizeof(aad),
        tampered,
        ct_written,
        plaintext_out,
        sizeof(plaintext_out),
        &pt_written);
    expect_status(st, MITHRIL_ERR_INTERNAL, "aead_decrypt rejects tampered ciphertext");

    expect_status(
        mithril_aead_encrypt(
            ctx,
            alg,
            key,
            key_len - 1u,
            nonce,
            nonce_len,
            aad,
            sizeof(aad),
            plaintext,
            sizeof(plaintext),
            ciphertext,
            sizeof(ciphertext),
            &ct_written),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "aead_encrypt rejects short key");

    expect_status(
        mithril_aead_decrypt(
            ctx,
            alg,
            key,
            key_len,
            nonce,
            nonce_len,
            aad,
            sizeof(aad),
            ciphertext,
            tag_len - 1u,
            plaintext_out,
            sizeof(plaintext_out),
            &pt_written),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "aead_decrypt rejects too short ciphertext");

    expect_status(
        mithril_aead_encrypt(
            ctx,
            999u,
            key,
            key_len,
            nonce,
            nonce_len,
            aad,
            sizeof(aad),
            plaintext,
            sizeof(plaintext),
            ciphertext,
            sizeof(ciphertext),
            &ct_written),
        MITHRIL_ERR_NOT_IMPLEMENTED,
        "aead_encrypt rejects unsupported algorithm");
}

static void test_kex_roundtrip(mithril_context *ctx) {
    const uint32_t alg = MITHRIL_KEX_ALG_X25519;

    uint8_t alice_pk[32] = {0};
    uint8_t alice_sk[32] = {0};
    uint8_t bob_pk[32] = {0};
    uint8_t bob_sk[32] = {0};
    uint8_t shared_ab[32] = {0};
    uint8_t shared_ba[32] = {0};
    size_t shared_ab_len = 0u;
    size_t shared_ba_len = 0u;

    size_t pk_len = mithril_kex_public_key_size(ctx, alg);
    size_t sk_len = mithril_kex_secret_key_size(ctx, alg);
    size_t ss_len = mithril_kex_shared_secret_size(ctx, alg);

    assert(pk_len == sizeof(alice_pk));
    assert(sk_len == sizeof(alice_sk));
    assert(ss_len == sizeof(shared_ab));

    expect_ok(
        mithril_kex_keypair(ctx, alg, alice_pk, pk_len, alice_sk, sk_len),
        "kex keypair alice");
    expect_ok(
        mithril_kex_keypair(ctx, alg, bob_pk, pk_len, bob_sk, sk_len),
        "kex keypair bob");

    expect_ok(
        mithril_kex_shared_secret(
            ctx,
            alg,
            alice_sk,
            sk_len,
            bob_pk,
            pk_len,
            shared_ab,
            sizeof(shared_ab),
            &shared_ab_len),
        "kex shared secret alice->bob");

    expect_ok(
        mithril_kex_shared_secret(
            ctx,
            alg,
            bob_sk,
            sk_len,
            alice_pk,
            pk_len,
            shared_ba,
            sizeof(shared_ba),
            &shared_ba_len),
        "kex shared secret bob->alice");

    assert(shared_ab_len == ss_len);
    assert(shared_ba_len == ss_len);
    assert(memcmp(shared_ab, shared_ba, ss_len) == 0);

    expect_status(
        mithril_kex_keypair(ctx, 999u, alice_pk, pk_len, alice_sk, sk_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "kex keypair rejects unsupported algorithm");

    expect_status(
        mithril_kex_shared_secret(
            ctx,
            alg,
            alice_sk,
            sk_len,
            bob_pk,
            pk_len - 1u,
            shared_ab,
            sizeof(shared_ab),
            &shared_ab_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "kex shared secret rejects short peer public key");

    expect_status(
        mithril_kex_shared_secret(
            ctx,
            alg,
            alice_sk,
            sk_len,
            bob_pk,
            pk_len,
            shared_ab,
            ss_len - 1u,
            &shared_ab_len),
        MITHRIL_ERR_INVALID_ARGUMENT,
        "kex shared secret rejects short output buffer");
}

int main(void) {
    mithril_context *ctx = NULL;
    mithril_status st = mithril_init(&ctx, NULL);

    if (st == MITHRIL_ERR_NOT_IMPLEMENTED) {
        puts("[SKIP] test_sodium_aead_kex (sodium provider not enabled at build time)");
        return 0;
    }

    expect_ok(st, "mithril_init");
    activate_sodium(ctx);

    test_aead_roundtrip(ctx);
    test_kex_roundtrip(ctx);

    mithril_shutdown(ctx);
    puts("[OK] test_sodium_aead_kex");
    return 0;
}
