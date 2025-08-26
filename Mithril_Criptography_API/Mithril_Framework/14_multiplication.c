//
//  14_multiplication.c
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 26-08-25.
//

#include <flint.h>
#include <fmpz.h>
#include "13_multiplication.h"

int fmpz_mul_safe(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    // Handle zero cases efficiently
    if (fmpz_is_zero(a) || fmpz_is_zero(b)) {
        fmpz_zero(result);
        return 0; // Success
    }
    
    // Perform multiplication using FLINT's optimized algorithm
    fmpz_mul(result, a, b);
    
    // Check for overflow (FLINT handles arbitrary precision automatically)
    // This is mainly for compatibility with your original interface
    return 0; // FLINT handles arbitrary precision, so no overflow
}

// Example usage function
void example_usage() {
    fmpz_t a, b, result;
    
    // Initialize FLINT integers
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(result);
    
    // Set values (example: a = 12345, b = 67890)
    fmpz_set_ui(a, 12345);
    fmpz_set_ui(b, 67890);
    
    // Perform multiplication
    int status = fmpz_mul_safe(result, a, b);
    
    if (status == 0) {
        // Print result using FLINT's modern printf
        flint_printf("Result: %{fmpz}\n", result);
    }
    
    // Clean up memory
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(result);
}
