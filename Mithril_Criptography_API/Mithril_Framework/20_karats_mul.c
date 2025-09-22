//
//  20_karatz_mul.c
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 22-09-25.
//

#include "19_karats_mul.h"
#include "FLINT/flint.h"
#include "FLINT/fmpz.h"
#include "FLINT.framework/Headers/gmp.h"

void flint_kmul(const fmpz_t a, const fmpz_t b, fmpz_t result) {
    /* FLINT 3.x automatically chooses optimal multiplication algorithm:
     * - Basecase for small operands
     * - Karatsuba for medium operands
     * - FFT for large operands
     * Uses optimized assembly code and SIMD when available */
    fmpz_mul(result, a, b);
}

void flint_kmul_limbs(const mp_limb_t *a_limbs, slong a_size,
                      const mp_limb_t *b_limbs, slong b_size,
                      fmpz_t result) {
    fmpz_t a, b;
    
    /* Initialize temporary FLINT integers */
    fmpz_init(a);
    fmpz_init(b);
    
    /* Convert limb arrays to FLINT integers */
    fmpz_set_ui_array(a, a_limbs, a_size);
    fmpz_set_ui_array(b, b_limbs, b_size);
    
    /* Use FLINT's optimized multiplication */
    fmpz_mul(result, a, b);
    
    /* Clean up */
    fmpz_clear(a);
    fmpz_clear(b);
}

/* Legacy wrapper for backward compatibility */
void kmul_compat(const mp_limb_t *aptr_l, const mp_limb_t *bptr_l,
                 slong len_a, slong len_b, fmpz_t p_l) {
    flint_kmul_limbs(aptr_l, len_a, bptr_l, len_b, p_l);
}
