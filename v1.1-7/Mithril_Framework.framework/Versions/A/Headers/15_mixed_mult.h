//
//  15_mixed_mult.h
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 03-09-25.
//

#ifndef MODERN_MULTIPLICATION_H
#define MODERN_MULTIPLICATION_H

#include "flint.h"
#include "fmpz.h"

/* Error codes matching the original C99 design */
#define E_FLINT_OK  0
#define E_FLINT_OFL 1

/* Function prototype using FLINT types */
int fmpz_mul_ui_mod(fmpz_t result, const fmpz_t a, ulong b, const fmpz_t modulus);

#endif
