//
//  06_mixed_addition.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#include "05_mixed_addition.h"
#include <stdio.h>
#include "flint.h"
#include "fmpz.h"

int main(void)
{
    fmpz_t a, result;
    ulong b;
    
    /* Initialize FLINT integers */
    fmpz_init(a);
    fmpz_init(result);
    
    /* Set some test values */
    fmpz_set_str(a, "123456789012345678901234567890", 10);
    b = 42;
    
    printf("Original large integer: ");
    fmpz_print(a);
    printf("\n");
    
    printf("Adding unsigned integer: %lu\n", b);
    
    /* Use FLINT's built-in mixed addition directly */
    fmpz_add_ui(result, a, b);
    
    printf("Result: ");
    fmpz_print(result);
    printf("\n");
    
    /* Clean up */
    fmpz_clear(a);
    fmpz_clear(result);
    
    return 0;
}
