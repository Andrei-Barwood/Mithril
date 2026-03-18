#ifndef MITHRIL_SIGN_H
#define MITHRIL_SIGN_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_api.h"
#include "mithril_error.h"

enum {
    MITHRIL_SIGN_ALG_ED25519 = 1u
};

#ifdef __cplusplus
extern "C" {
#endif

size_t mithril_sign_public_key_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_sign_secret_key_size(mithril_context *ctx, uint32_t algorithm);
size_t mithril_sign_signature_size(mithril_context *ctx, uint32_t algorithm);

mithril_status mithril_sign_keypair(
    mithril_context *ctx,
    uint32_t algorithm,
    uint8_t *public_key,
    size_t public_key_len,
    uint8_t *secret_key,
    size_t secret_key_len);

mithril_status mithril_sign_detached(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *secret_key,
    size_t secret_key_len,
    uint8_t *signature,
    size_t signature_len,
    size_t *written_len);

mithril_status mithril_sign_verify_detached(
    mithril_context *ctx,
    uint32_t algorithm,
    const uint8_t *message,
    size_t message_len,
    const uint8_t *public_key,
    size_t public_key_len,
    const uint8_t *signature,
    size_t signature_len);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_SIGN_H */
