//
//  13_multiplication.h
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 26-08-25.
//

#ifndef MULTIPLICATION_H
#define MULTIPLICATION_H

#include <flint.h>
#include <fmpz.h>

// Modern FLINT function declaration
int fmpz_mul_safe(fmpz_t result, const fmpz_t a, const fmpz_t b);

#endif
