//
// 03_subtraction.h
// Mithril_Criptography_API
//
// Created by Andres Barbudo on 21-08-25.
//

#ifndef LIBSODIUM_SUBTRACTION_H
#define LIBSODIUM_SUBTRACTION_H

#include <sodium.h>
#include <stddef.h>

// Return codes
#define E_SODIUM_OK 0
#define E_SODIUM_UFL 1  // Underflow

// Function to perform subtraction with underflow handling
// Numbers are in little-endian format
int sodium_sub_with_underflow(unsigned char *result, 
                               const unsigned char *a, 
                               const unsigned char *b, 
                               size_t len);

#endif
