#include "provider_sodium.h"

#include "mithril/mithril_provider.h"
#include "provider_sodium_internal.h"

static mithril_provider_sodium_state g_sodium_state = {0};

static const mithril_provider_ops g_sodium_ops = {
    .on_activate = mithril_provider_sodium_on_activate,
    .on_deactivate = mithril_provider_sodium_on_deactivate,
    .get_capabilities = mithril_provider_sodium_get_capabilities
};

static const mithril_provider_crypto_ops g_sodium_crypto_ops = {
    .rng_fill = mithril_provider_sodium_rng_fill,
    .hash_compute = mithril_provider_sodium_hash_compute,
    .hash_digest_size = mithril_provider_sodium_hash_digest_size,
    .aead_encrypt = mithril_provider_sodium_aead_encrypt,
    .aead_decrypt = mithril_provider_sodium_aead_decrypt,
    .aead_key_size = mithril_provider_sodium_aead_key_size,
    .aead_nonce_size = mithril_provider_sodium_aead_nonce_size,
    .aead_tag_size = mithril_provider_sodium_aead_tag_size,
    .kex_keypair = mithril_provider_sodium_kex_keypair,
    .kex_shared_secret = mithril_provider_sodium_kex_shared_secret,
    .kex_public_key_size = mithril_provider_sodium_kex_public_key_size,
    .kex_secret_key_size = mithril_provider_sodium_kex_secret_key_size,
    .kex_shared_secret_size = mithril_provider_sodium_kex_shared_secret_size,
    .sign_keypair = mithril_provider_sodium_sign_keypair,
    .sign_detached = mithril_provider_sodium_sign_detached,
    .sign_verify_detached = mithril_provider_sodium_sign_verify_detached,
    .sign_public_key_size = mithril_provider_sodium_sign_public_key_size,
    .sign_secret_key_size = mithril_provider_sodium_sign_secret_key_size,
    .sign_signature_size = mithril_provider_sodium_sign_signature_size
};

mithril_status mithril_provider_sodium_register(mithril_context *ctx) {
    mithril_provider_descriptor descriptor;

    descriptor.name = "sodium";
    descriptor.abi_version = MITHRIL_PROVIDER_ABI_VERSION;
    descriptor.declared_capabilities = MITHRIL_CAP_RNG | MITHRIL_CAP_HASH | MITHRIL_CAP_AEAD | MITHRIL_CAP_KEX | MITHRIL_CAP_SIGN;
    descriptor.ops = &g_sodium_ops;
    descriptor.crypto_ops = &g_sodium_crypto_ops;
    descriptor.user_data = &g_sodium_state;

    return mithril_provider_register(ctx, &descriptor);
}
