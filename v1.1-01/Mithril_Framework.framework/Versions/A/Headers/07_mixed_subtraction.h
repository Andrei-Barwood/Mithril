//
//  07_mixed_subtraction.h
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#ifndef MODERN_SUBTRACTION_H
#define MODERN_SUBTRACTION_H

#include <flint.h>
#include <fmpz.h>

/**
 * Subtract an unsigned short from a FLINT integer
 * @param result: The fmpz_t to store the result (a - b)
 * @param a: The fmpz_t integer to subtract from
 * @param b: The unsigned short to subtract
 * @return: 0 on success, error code on failure
 */
int fmpz_sub_ushort(fmpz_t result, const fmpz_t a, unsigned short b);

#endif
