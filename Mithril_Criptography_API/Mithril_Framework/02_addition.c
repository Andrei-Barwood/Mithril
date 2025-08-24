//
//  02_addition.c
//  Modern FLINT Implementation
//
//  Created by Andres Barbudo on 20-08-25.
//
/*
#include "flint.h"
#include "fmpz.h"
#include "stdio.h"
*/
/**
 * Modern FLINT implementation of big integer addition
 * @param result - output: sum of a and b
 * @param a - first operand
 * @param b - second operand
 * @return 0 on success, 1 if overflow would have occurred in the old system
 */

/*
int fmpz_add_modern(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    // FLINT handles all overflow automatically - no manual overflow checking needed
    fmpz_add(result, a, b);
    
    // FLINT 3.x automatically handles arbitrary precision
    // Return 0 to indicate success (no overflow in modern FLINT)
    return 0;
}
*/
/**
 * Alternative implementation with explicit overflow detection
 * for compatibility with legacy code that expects overflow checking
 */

/*
int fmpz_add_with_legacy_overflow_check(fmpz_t result, const fmpz_t a, const fmpz_t b,
                                       size_t max_bits) {
    fmpz_add(result, a, b);
    
    // Check if result exceeds the specified bit limit (for legacy compatibility)
    if (fmpz_bits(result) > max_bits) {
        // Could truncate here if needed for legacy behavior
        // fmpz_tdiv_r_2exp(result, result, max_bits);
        return 1; // Indicate "overflow" occurred
    }
    
    return 0;
}
*/
/**
 * Simple usage example
 */
/*
int main() {
    fmpz_t a, b, sum;
    
    // Initialize FLINT integers
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(sum);
    
    // Set some values
    fmpz_set_str(a, "123456789012345678901234567890", 10);
    fmpz_set_str(b, "987654321098765432109876543210", 10);
    
    // Perform addition
    int result = fmpz_add_modern(sum, a, b);
    
    // Print result
    flint_printf("Result: ");
    fmpz_print(sum);
    flint_printf("\nStatus: %d\n", result);
    
    // Clean up
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(sum);
    
    return 0;
}
*/
