#ifndef MITHRIL_PROVIDER_C23_INTERNAL_H
#define MITHRIL_PROVIDER_C23_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "mithril/mithril_capabilities.h"
#include "mithril/mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mithril_provider_c23_state {
    int initialized;
} mithril_provider_c23_state;

mithril_status mithril_provider_c23_on_activate(void *user_data);
void mithril_provider_c23_on_deactivate(void *user_data);
mithril_status mithril_provider_c23_get_capabilities(void *user_data, mithril_capabilities *out_caps);

mithril_status mithril_provider_c23_bigint_add(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

mithril_status mithril_provider_c23_bigint_sub(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

mithril_status mithril_provider_c23_bigint_mul(
    void *user_data,
    const uint8_t *a,
    size_t a_len,
    const uint8_t *b,
    size_t b_len,
    uint8_t *out,
    size_t out_len,
    size_t *written_len);

mithril_status mithril_provider_c23_modarith_add_mod(
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

mithril_status mithril_provider_c23_modarith_mul_mod(
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

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_PROVIDER_C23_INTERNAL_H */
