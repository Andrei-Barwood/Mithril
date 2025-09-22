//
//  19_karatz_mul.h
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 22-09-25.
//

#ifndef KARATS_MUL_H
#define KARATS_MUL_H

#include "FLINT/flint.h"
#include "FLINT.framework/Headers/gmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Modern FLINT-based multiplication function */
void flint_kmul(const fmpz_t a, const fmpz_t b, fmpz_t result);

/* Alternative function for working with limb arrays directly */
void flint_kmul_limbs(const mp_limb_t *a_limbs, slong a_size,
                      const mp_limb_t *b_limbs, slong b_size,
                      fmpz_t result);

#ifdef __cplusplus
}
#endif

#endif /* KARATS_MUL_H */
