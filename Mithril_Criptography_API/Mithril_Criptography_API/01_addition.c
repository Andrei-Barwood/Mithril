//
//  01_addition.c
//  Mithril Cryptography API
//
//  Created by Andres Barbudo on 17-08-25.
//

#include "stdio.h"
#include "stdlib.h"
#include "01_addition.h"

int add_l(const fmpz_t a, const fmpz_t b, fmpz_t s)
{
    fmpz_add(s, a, b);
    return 0;
}
