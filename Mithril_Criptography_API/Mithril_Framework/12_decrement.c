//
//  12_decrement.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#include "11_decrement.h"

fmpz_error_t fmpz_dec(fmpz_t a) {
    // Check for underflow - if a is zero, decrementing would make it negative
    if (fmpz_is_zero(a)) {
        // Set to maximum value (equivalent to your setmax_l behavior)
        // For unsigned arithmetic simulation, set to -1 or handle as error
        fmpz_set_si(a, -1);
        return E_FMPZ_UFL;
    }
    
    // Simply subtract 1 using the correct FLINT function
    fmpz_sub_ui(a, a, 1);
    return E_FMPZ_OK;
}
