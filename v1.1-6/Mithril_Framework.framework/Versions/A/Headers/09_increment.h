//
//  09_increment.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#ifndef INCREMENT_H
#define INCREMENT_H

#include <flint.h>
#include <fmpz.h>

/* Error codes matching your original interface */
#define E_FMPZ_OK       0
#define E_FMPZ_OFL      1

/* Function to increment an fmpz_t integer */
int fmpz_inc(fmpz_t result, const fmpz_t a);

/* In-place increment function */
int fmpz_inc_inplace(fmpz_t a);

/* Increment with modulus (equivalent to your overflow behavior) */
int fmpz_inc_mod(fmpz_t result, const fmpz_t a, const fmpz_t mod);

#endif
