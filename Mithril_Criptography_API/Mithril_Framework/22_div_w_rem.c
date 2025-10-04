//
//  22_div_w_rem.c
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 04-10-25.
//

#include <flint/flint.h>
#include <flint/fmpz.h>

int div_fmpz(fmpz_t quot, fmpz_t rem, const fmpz_t dividend, const fmpz_t divisor)
{
    // Check for division by zero
    if (fmpz_is_zero(divisor)) {
        return -1;  // Error: division by zero
    }
    
    // Handle zero dividend
    if (fmpz_is_zero(dividend)) {
        fmpz_zero(quot);
        fmpz_zero(rem);
        return 0;  // Success
    }
    
    // Perform division with remainder using FLINT's built-in function
    // fmpz_tdiv_qr performs truncated division (rounds toward zero)
    fmpz_tdiv_qr(quot, rem, dividend, divisor);
    
    return 0;  // Success
}
