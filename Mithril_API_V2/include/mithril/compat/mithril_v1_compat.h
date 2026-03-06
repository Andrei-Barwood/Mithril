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

#ifndef E_FMPZ_OK
#define E_FMPZ_OK 0
#endif

#ifndef E_FMPZ_OFL
#define E_FMPZ_OFL 1
#endif

#ifndef E_FMPZ_UFL
#define E_FMPZ_UFL -1
#endif

#ifndef E_FLINT_OK
#define E_FLINT_OK 0
#endif

#ifndef E_FLINT_OFL
#define E_FLINT_OFL 1
#endif

int fmpz_mul_safe(fmpz_t result, const fmpz_t a, const fmpz_t b);
int fmpz_square_safe(fmpz_t result, const fmpz_t input);
int fmpz_inc(fmpz_t result, const fmpz_t a);
int fmpz_inc_inplace(fmpz_t a);
int fmpz_inc_mod(fmpz_t result, const fmpz_t a, const fmpz_t mod);
int fmpz_dec(fmpz_t a);
int fmpz_sub_ushort(fmpz_t result, const fmpz_t a, unsigned short b);
int fmpz_mul_ui_mod(fmpz_t result, const fmpz_t a, ulong b, const fmpz_t modulus);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_V1_COMPAT_H */
