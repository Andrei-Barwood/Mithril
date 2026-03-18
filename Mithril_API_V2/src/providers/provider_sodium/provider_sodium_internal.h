#ifndef MITHRIL_PROVIDER_SODIUM_INTERNAL_H
#define MITHRIL_PROVIDER_SODIUM_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "mithril/mithril_capabilities.h"
#include "mithril/mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mithril_provider_sodium_state {
    int initialized;
} mithril_provider_sodium_state;

mithril_status mithril_provider_sodium_on_activate(void *user_data);
void mithril_provider_sodium_on_deactivate(void *user_data);
mithril_status mithril_provider_sodium_get_capabilities(void *user_data, mithril_capabilities *out_caps);

mithril_status mithril_provider_sodium_rng_fill(void *user_data, uint8_t *out, size_t out_len);
mithril_status mithril_provider_sodium_hash_compute(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *input,
    size_t input_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);
size_t mithril_provider_sodium_hash_digest_size(void *user_data, uint32_t algorithm);

mithril_status mithril_provider_sodium_aead_encrypt(
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

mithril_status mithril_provider_sodium_aead_decrypt(
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

size_t mithril_provider_sodium_aead_key_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_aead_nonce_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_aead_tag_size(void *user_data, uint32_t algorithm);

mithril_status mithril_provider_sodium_kex_keypair(
    void *user_data,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len);

mithril_status mithril_provider_sodium_kex_shared_secret(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *my_secret_key,
    size_t my_secret_key_len,
    const uint8_t *peer_public_key,
    size_t peer_public_key_len,
    uint8_t *shared_secret,
    size_t shared_secret_len,
    size_t *written_len);

size_t mithril_provider_sodium_kex_public_key_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_kex_secret_key_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_kex_shared_secret_size(void *user_data, uint32_t algorithm);

mithril_status mithril_provider_sodium_sign_keypair(
    void *user_data,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len);

mithril_status mithril_provider_sodium_sign_detached(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *secret_key,
    size_t secret_key_len,
    uint8_t *signature,
    size_t signature_len,
    size_t *written_len);

mithril_status mithril_provider_sodium_sign_verify_detached(
    void *user_data,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *public_key,
    size_t public_key_len,
    const uint8_t *signature,
    size_t signature_len);

size_t mithril_provider_sodium_sign_public_key_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_sign_secret_key_size(void *user_data, uint32_t algorithm);
size_t mithril_provider_sodium_sign_signature_size(void *user_data, uint32_t algorithm);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_PROVIDER_SODIUM_INTERNAL_H */
