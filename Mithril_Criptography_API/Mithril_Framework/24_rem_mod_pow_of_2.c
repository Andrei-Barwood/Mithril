#include <flint/fmpz.h>

void rem_mod_pow_of_2(const fmpz_t x, ulong k, fmpz_t res)
{
    fmpz_t mod;
    fmpz_init(mod);

    // Set mod = 2^k
    fmpz_set_ui(mod, 1UL);
    fmpz_mul_2exp(mod, mod, k);

    // res = x % 2^k
    fmpz_mod(res, x, mod);

    fmpz_clear(mod);
}
