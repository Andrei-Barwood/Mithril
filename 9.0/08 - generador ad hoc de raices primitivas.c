#include "flint.h"

int primroot_l (CLINT a_l, unsigned int noofprimes, clint *primes_l[]) {
	CLINT p_l, t_l, junk_l;
	ULONG i;
	if(ISODD_L (primes_l[0])) {
		return -1;
	}

	cpy_l (p_l, primes_l[0]);
	inc_l (p_l);
	SETONE_L (a_l);

	do {
		inc_l (a_l);

		if (issqr_l (a_l, t_l)) {
			inc_l (a_l);
		}
		i = 1;

		do {
			div_l (primes_l[0], primes_l[i++], t_l, junk_l);
			mexpkm_l (a_l, t_l, t_l, p_l);
		}

		while ((i <= noofprimes) && !EQONE_L (t_l));
	}
	while (EQONE_L (t_l));

	return E_CLINT_OK;
}