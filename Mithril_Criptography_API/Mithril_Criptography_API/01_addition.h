//
//  01_addition.h
//  Mithril Cryptography API
//
//  Created by Andres Barbudo on 17-08-25.
//

#ifndef MATH_OPERATIONS_H
#define MATH_OPERATIONS_H

#include <fmpz.h>

#ifndef MITHRIL_V1_DEPRECATED
#if defined(__clang__) || defined(__GNUC__)
#define MITHRIL_V1_DEPRECATED __attribute__((deprecated("Mithril v1 is deprecated; migrate to Mithril_API_V2.")))
#else
#define MITHRIL_V1_DEPRECATED
#endif
#endif

MITHRIL_V1_DEPRECATED int add_l(const fmpz_t a, const fmpz_t b, fmpz_t s);

#endif // MATH_OPERATIONS_H
