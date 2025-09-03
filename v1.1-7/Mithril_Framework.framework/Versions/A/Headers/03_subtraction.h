//
//  03_subtraction.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#ifndef MODERN_SUBTRACTION_H
#define MODERN_SUBTRACTION_H

#include <fmpz.h>

// Return codes
#define E_FLINT_OK    0
#define E_FLINT_UFL   1  // Underflow

// Function to perform subtraction with underflow handling
int fmpz_sub_with_underflow(fmpz_t result, const fmpz_t a, const fmpz_t b);

#endif
