//
//  11_decrement.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#ifndef FMPZ_DECREMENT_H
#define FMPZ_DECREMENT_H

#include "flint.h"
#include "fmpz.h"

// Error codes for consistency with your original design
typedef enum {
    E_FMPZ_OK = 0,
    E_FMPZ_UFL = -1  // Underflow error
} fmpz_error_t;

// Function declaration
fmpz_error_t fmpz_dec(fmpz_t a);

#endif
