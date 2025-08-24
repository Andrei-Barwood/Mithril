//
//  04_subtraction.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#include "03_subtraction.h"

int fmpz_sub_with_underflow(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    int underflow = E_FLINT_OK;
    
    // Check if a < b (would cause underflow in unsigned arithmetic)
    if (fmpz_cmp(a, b) < 0) {
        // Handle underflow case: compute |b - a| - 1
        // This mimics the original CLINT behavior
        fmpz_sub(result, b, a);  // result = b - a
        fmpz_sub_ui(result, result, 1);  // result = (b - a) - 1
        underflow = E_FLINT_UFL;
    } else {
        // Normal case: a >= b
        fmpz_sub(result, a, b);  // result = a - b
    }
    
    return underflow;
}
