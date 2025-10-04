//
//  21_div_w_rem.h
//  Mithril_Cryptography_API
//
//  Created by Andres Barbudo on 04-10-25.
//

#ifndef DIVISION_WITH_REMAINDER_H
#define DIVISION_WITH_REMAINDER_H

#include <flint/flint.h>
#include <flint/fmpz.h>

int div_fmpz(fmpz_t quot, fmpz_t rem, const fmpz_t dividend, const fmpz_t divisor);

#endif
