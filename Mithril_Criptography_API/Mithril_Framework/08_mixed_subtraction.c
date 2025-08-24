//
//  08_mixed_subtraction.c
//  Mithril_Criptography_API
//
//  Created by Andres Barbudo on 21-08-25.
//

#include "07_mixed_subtraction.h"

int fmpz_sub_ushort(fmpz_t result, const fmpz_t a, unsigned short b) {
    // Create a temporary fmpz for the unsigned short
    fmpz_t temp;
    fmpz_init(temp);
    
    // Set the temporary value
    fmpz_set_ui(temp, (unsigned long)b);
    
    // Perform subtraction: result = a - temp
    fmpz_sub(result, a, temp);
    
    // Clean up
    fmpz_clear(temp);
    
    return 0;
}
