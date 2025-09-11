//
//  17_squaring.h
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 11-09-25.
//

#ifndef FLINT_SQUARING_H
#define FLINT_SQUARING_H

#include <flint.h>
#include <fmpz.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Square an arbitrary precision integer using FL```
 *
 * @param result   Output parameter for the```uared result
 * @param input    Input integer to square
 * @return         0 on success, non-zero on error
 */
int fmpz_square_safe(fmpz_t result, const fmpz_t input);

/**
 * Square an arbitrary precision integer (inline version)
 * Direct wrapper around FLINT's optimized squaring
 */
static inline void fmpz_square(fmpz_t result, const fmpz_t input)
{
    fmpz_mul(result, input, input);
}

#ifdef __cplusplus
}
#endif

#endif /* FLINT_SQUARING_H */
