#ifndef MITHRIL_V1_COMPAT_H
#define MITHRIL_V1_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MITHRIL_SCALAR_BYTES
#define MITHRIL_SCALAR_BYTES 32u
#endif

#ifndef E_SODIUM_OK
#define E_SODIUM_OK 0
#endif

#ifndef E_SODIUM_UFL
#define E_SODIUM_UFL 1
#endif

int mithril_v1_compat_init(void);
void mithril_v1_compat_shutdown(void);

int mithril_add_scalar(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]);

int mithril_add_mod_l(
    uint8_t result[MITHRIL_SCALAR_BYTES],
    const uint8_t a[MITHRIL_SCALAR_BYTES],
    const uint8_t b[MITHRIL_SCALAR_BYTES]);

int mithril_add_constant_time(
    uint8_t *result,
    const uint8_t *a,
    const uint8_t *b,
    size_t len);

int sodium_sub_with_underflow(
    unsigned char *result,
    const unsigned char *a,
    const unsigned char *b,
    size_t len);

#if defined(MITHRIL_USE_FLINT)
#include <flint/fmpz.h>

int fmpz_mul_safe(fmpz_t result, const fmpz_t a, const fmpz_t b);
int fmpz_square_safe(fmpz_t result, const fmpz_t input);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_V1_COMPAT_H */
