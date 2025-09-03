//
//  16_mixed_mult.c
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 03-09-25.
//

#include "15_mixed_mult.h"

/**
 * Multiply a FLINT integer by an unsigned long with optional modular reduction
 *
 * @param result   Output: the product a * b (possibly reduced)
 * @param a        Input: the multi-precision integer
 * @param b        Input: the single word multiplier
 * @param modulus  Input: modulus for reduction (can be NULL for no reduction)
 * @return E_FLINT_OK on success, E_FLINT_OFL if overflow handling was needed
 */
int fmpz_mul_ui_mod(fmpz_t result, const fmpz_t a, ulong b, const fmpz_t modulus)
{
    int overflow_flag = E_FLINT_OK;
    
    /* Handle zero cases efficiently */
    if (fmpz_is_zero(a) || b == 0) {
        fmpz_zero(result);
        return E_FLINT_OK;
    }
    
    /* Perform the multiplication using FLINT's optimized function */
    fmpz_mul_ui(result, a, b);
    
    /* Apply modular reduction if modulus is provided */
    if (modulus != NULL && !fmpz_is_zero(modulus)) {
        /* Check if reduction was necessary (equivalent to overflow in C99) */
        if (fmpz_cmp(result, modulus) >= 0) {
            overflow_flag = E_FLINT_OFL;
            fmpz_mod(result, result, modulus);
        }
    }
    
    return overflow_flag;
}
