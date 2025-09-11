//
//  18_squaring.c
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 11-09-25.
//

#include "17_squaring.h"
#include <flint.h>
#include <fmpz.h>

/**
 * Square an arbitrary precision integer with error handling
 */
int fmpz_square_safe(fmpz_t result, const fmpz_t input)
{
    // Check for null pointers (basic error checking)
    if (!result || !input) {
        return -1;
    }
    
    // Handle zero case explicitly (though FLINT handles this correctly)
    if (fmpz_is_zero(input)) {
        fmpz_zero(result);
        return 0;
    }
    
    // Use FLINT's optimized multiplication for squaring```  // FLINT automatically detects when both operands are the same```  // and uses specialized squaring algorithms```ternally
    fmpz_mul(result, input, input);
    
    // FLINT handles arbitrary precision automatically,
    // so no overflow checking is needed
    return 0;
}
