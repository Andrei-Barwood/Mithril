//
//  10_increment.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#include "09_increment.h"

int fmpz_inc(fmpz_t result, const fmpz_t a)
{
    /* Simply add 1 to the input value */
    fmpz_add_ui(result, a, 1UL);
    return E_FMPZ_OK;
}

int fmpz_inc_inplace(fmpz_t a)
{
    /* In-place increment - add 1 to the value */
    fmpz_add_ui(a, a, 1UL);
    return E_FMPZ_OK;
}

int fmpz_inc_mod(fmpz_t result, const fmpz_t a, const fmpz_t mod)
{
    fmpz_t temp;
    fmpz_init(temp);
    
    /* Increment the value */
    fmpz_add_ui(temp, a, 1UL);
    
    /* Apply modular reduction */
    fmpz_mod(result, temp, mod);
    
    fmpz_clear(temp);
    return E_FMPZ_OK;
}
