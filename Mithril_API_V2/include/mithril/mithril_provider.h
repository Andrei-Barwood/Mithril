#ifndef MITHRIL_PROVIDER_H
#define MITHRIL_PROVIDER_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_capabilities.h"
#include "mithril_error.h"
#include "mithril_provider_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MITHRIL_PROVIDER_ABI_VERSION 1u

typedef struct mithril_provider_crypto_ops {
    mithril_status (*rng_fill)(void *user_data, uint8_t *out, size_t out_len);

    mithril_status (*hash_compute)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *input,
        size_t input_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);
    size_t (*hash_digest_size)(void *user_data, uint32_t algorithm);

    mithril_status (*aead_encrypt)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *key,
        size_t key_len,
        const uint8_t *nonce,
        size_t nonce_len,
        const uint8_t *aad,
        size_t aad_len,
        const uint8_t *plaintext,
        size_t plaintext_len,
        uint8_t *ciphertext,
        size_t ciphertext_len,
        size_t *written_len);

    mithril_status (*aead_decrypt)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *key,
        size_t key_len,
        const uint8_t *nonce,
        size_t nonce_len,
        const uint8_t *aad,
        size_t aad_len,
        const uint8_t *ciphertext,
        size_t ciphertext_len,
        uint8_t *plaintext,
        size_t plaintext_len,
        size_t *written_len);

    size_t (*aead_key_size)(void *user_data, uint32_t algorithm);
    size_t (*aead_nonce_size)(void *user_data, uint32_t algorithm);
    size_t (*aead_tag_size)(void *user_data, uint32_t algorithm);

    mithril_status (*kex_keypair)(
        void *user_data,
        uint32_t algorithm,
        uint8_t *public_key,
        size_t public_key_len,
        uint8_t *secret_key,
        size_t secret_key_len);

    mithril_status (*kex_shared_secret)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *my_secret_key,
        size_t my_secret_key_len,
        const uint8_t *peer_public_key,
        size_t peer_public_key_len,
        uint8_t *shared_secret,
        size_t shared_secret_len,
        size_t *written_len);

    size_t (*kex_public_key_size)(void *user_data, uint32_t algorithm);
    size_t (*kex_secret_key_size)(void *user_data, uint32_t algorithm);
    size_t (*kex_shared_secret_size)(void *user_data, uint32_t algorithm);

    mithril_status (*sign_keypair)(
        void *user_data,
        uint32_t algorithm,
        uint8_t *public_key,
        size_t public_key_len,
        uint8_t *secret_key,
        size_t secret_key_len);

    mithril_status (*sign_detached)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *message,
        size_t message_len,
        const uint8_t *secret_key,
        size_t secret_key_len,
        uint8_t *signature,
        size_t signature_len,
        size_t *written_len);

    mithril_status (*sign_verify_detached)(
        void *user_data,
        uint32_t algorithm,
        const uint8_t *message,
        size_t message_len,
        const uint8_t *public_key,
        size_t public_key_len,
        const uint8_t *signature,
        size_t signature_len);

    size_t (*sign_public_key_size)(void *user_data, uint32_t algorithm);
    size_t (*sign_secret_key_size)(void *user_data, uint32_t algorithm);
    size_t (*sign_signature_size)(void *user_data, uint32_t algorithm);

    mithril_status (*bigint_add)(
        void *user_data,
        const uint8_t *a,
        size_t a_len,
        const uint8_t *b,
        size_t b_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);

    mithril_status (*bigint_sub)(
        void *user_data,
        const uint8_t *a,
        size_t a_len,
        const uint8_t *b,
        size_t b_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);

    mithril_status (*bigint_mul)(
        void *user_data,
        const uint8_t *a,
        size_t a_len,
        const uint8_t *b,
        size_t b_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);

    mithril_status (*modarith_add_mod)(
        void *user_data,
        const uint8_t *a,
        size_t a_len,
        const uint8_t *b,
        size_t b_len,
        const uint8_t *modulus,
        size_t modulus_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);

    mithril_status (*modarith_mul_mod)(
        void *user_data,
        const uint8_t *a,
        size_t a_len,
        const uint8_t *b,
        size_t b_len,
        const uint8_t *modulus,
        size_t modulus_len,
        uint8_t *out,
        size_t out_len,
        size_t *written_len);
} mithril_provider_crypto_ops;

typedef struct mithril_provider_descriptor {
    const char *name;
    uint32_t abi_version;
    uint64_t declared_capabilities;
    const mithril_provider_ops *ops;
    const mithril_provider_crypto_ops *crypto_ops;
    void *user_data;
} mithril_provider_descriptor;

mithril_status mithril_provider_register(mithril_context *ctx, const mithril_provider_descriptor *descriptor);
mithril_status mithril_provider_activate(mithril_context *ctx, const char *name);
const char *mithril_provider_active_name(const mithril_context *ctx);
uint32_t mithril_provider_count(const mithril_context *ctx);
mithril_status mithril_provider_get_capabilities(const mithril_context *ctx, mithril_capabilities *out_caps);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_PROVIDER_H */
