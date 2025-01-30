#include "flint.h"

void and_l (CLINT a_l, CLINT b_l, CLINT c_l) {
	CLINT d_l:
	clint *r_l, *s_l, *t_l;
	clint *lastptr_l;

	if (DIGITS_L (a_l) < DIGITS_L (b_l)) {
		r_l = LSDPTR_L (b_l);
		s_l = LSDPTR_L (a_l);
		lastptr_l = MSDPTR_L (a_l);
	}
	else {
		r_l = LSDPTR_L (a_l);
		s_l = LSDPTR_L (b_l);
		lastptr_l = MSDPTR_L (b_l);
	}
	t_l = LSDPTR_L (d_l);
	SETDIGITS_L (d_l, DIGITS_L (s_l -1));
	while (s_l <= lastptr_l) {
		*t_l++ = *r_l++ & *s_l++;
	}
	cpy_l (c_l, d_l);
}