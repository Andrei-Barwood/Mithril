//
// 04_subtraction.c
// Mithril_Criptography_API
//
// Created by Andres Barbudo on 21-08-25.
//

#include "03_subtraction.h"
#include <string.h>

int sodium_sub_with_underflow(unsigned char *result, 
                               const unsigned char *a, 
                               const unsigned char *b, 
                               size_t len) {
    int underflow = E_SODIUM_OK;
    
    // Check if a < b (would cause underflow in unsigned arithmetic)
    if (sodium_compare(a, b, len) < 0) {
        // Handle underflow case: compute (b - a) - 1
        // This mimics the original CLINT/FLINT behavior
        
        // Create temporary buffers
        unsigned char *temp = sodium_malloc(len);
        unsigned char one[len];
        
        if (temp == NULL) {
            return -1; // Memory allocation error
        }
        
        // Initialize one = 1 in little-endian format
        memset(one, 0, len);
        one[0] = 1;
        
        // Copy b to temp
        memcpy(temp, b, len);
        
        // temp = b - a
        sodium_sub(temp, a, len);
        
        // result = temp - 1 = (b - a) - 1
        memcpy(result, temp, len);
        sodium_sub(result, one, len);
        
        sodium_free(temp);
        underflow = E_SODIUM_UFL;
        
    } else {
        // Normal case: a >= b
        // Copy a to result first (sodium_sub modifies first argument)
        memcpy(result, a, len);
        
        // result = a - b
        sodium_sub(result, b, len);
    }
    
    return underflow;
}
