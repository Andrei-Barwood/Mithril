#ifndef MITHRIL_AEAD_H
#define MITHRIL_AEAD_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

enum {
    MITHRIL_AEAD_ALG_XCHACHA20POLY1305_IETF = 1u
};

#ifdef __cplusplus
extern "C" {
#endif

size_t mithril_aead_key_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_aead_nonce_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_aead_tag_size(mithril_context *ctx, uint32_t algorithm);

mithril_status mithril_aead_encrypt(
    mithril_context *ctx,
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

mithril_status mithril_aead_decrypt(
    mithril_context *ctx,
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

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_AEAD_H */
