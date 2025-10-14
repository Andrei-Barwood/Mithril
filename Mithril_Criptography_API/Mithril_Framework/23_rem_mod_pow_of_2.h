#include <flint/fmpz.h>

// Returns the remainder of x mod (2^k), stores in res
void rem_mod_pow_of_2(const fmpz_t x, ulong k, fmpz_t res);
