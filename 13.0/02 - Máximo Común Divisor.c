#include "flint.h"

void gcd_l (CLINT aa_l, CLINT bb_l, CLINT cc_l) {
	CLINT a_l, b_l, r_l, t_l;
	unsigned int k = 0;
	int sign_of_t;

	if (LT_L (aa_l, bb_l)) {
		cpy_l (a_l, bb_l);
		cpy_l (b_l, aa_l);
	}
	else {
		cpy_l (a_l, aa_l);
		cpy_l (b_l, bb_l);
	}
	if (EQZ_L (b_l)) {
		cpy_l (cc_l, a_l);
		return;
	}

	(void) div_l (a_l, b_l, t_l, r_l);
	cpy_l (a_l, b_l);
	cpy_l (b_l, r_l);

	if (EQZ_L (b_l)) {
		cpy_l (cc_l, a_l);
		return;
	}

	while (ISEVEN_L (a_l) && ISEVEN_L (b_l)) {
		++k;
		shr_l (a_l);
		shr_l (b_l);
	}

	while (ISEVEN_L (a_l)) {
		shr_l (a_l);
	}
	while (ISEVEN_L (b_l)) {
		shr_l (b_l);
	}

	do {
		if (GE_L (a_l, b_l)) {
			sub_l (a_l, b_l, t_l);
			sign_of_t = 1;
		}
		else {
			sub_l (b_l, a_l, t_l);
			sign_of_t = -1;
		}
		if (EQZ_L (t_l)) {
			cpy_l (cc_l, a_l);
			shift_l (cc_l, (long int) k); /* cc_l <- cc_l*2**k */
			return;
		}
		while (ISEVEN_L (t_l)) {
			shr_l (t_l);
		}
		if (-1 == sign_of_t) {
			cpy_l (b_l, t_l);
		}
		else {
			cpy_l (a_l, t_l);
		}
	}
	while (1);
}