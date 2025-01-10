#include "flint.h"

void mulmon_l (CLINT a_l, CLINT b_l, CLINT n_l, USHORT nprime, USHORT logB_r, CLINT P_L) {
	CLINTD t_l;
	clint *tptr_l, *nptr_l, *tiptr_l, *lasttnptr, *lastnptr;
	ULONG carry;
	USHORT mi;
	int i;

	mult (a_l, b_l, t_l);
	lasttnptr = t_l + DIGITS_L (n_l);
	lastnptr = MSDPTR_L (n_l);

	for (i = DIGITS_L (t_l) + 1; i <= (DIGITS_L (n_l) << 1); i++) {
		t_l[i] = 0;
	}
	SETDIGITS_L (t_l, MAX (DIGITS_L (t_l), DIGITS_L (n_l) << 1));



	for (tptr_l = LSDPTR_L (t_l); tptr_l <= lasttnptr; tptr_l++) {
		carry = 0;
		mi = (USHORT)((ULONG)nprime * (ULONG)*tptr_l);


		for (nptr_l = LSDPTR_L (n_l), tiptr_l = tptr_l; nptr_l <= lastnptr; nptr_l++, tiptr_l++) {
			*tiptr_l = (USHORT)(carry = (ULONG)mi * (ULONG)*nptr_l + (ULONG)*tiptr_l + (ULONG)(USHORT)(carry >> BITPERDGT));
		}

		for (;
				((carry >> BITPERDGT) > 0) && tiptr_l <= MSDPTR_L (t_l); tiptr_l++) {
			*tiptr_l = (USHORT)(carry = (ULONG)*tiptr_l +
								(ULONG)(USHORT)(carry >> BITPERDGT));
		}
		if (((carry >> BITPERDGT) > 0)) {
			*tiptr_l = (USHORT)(carry >> BITPERDGT);
			INCDIGITS_L (t_l);
		}

	}

	

	tptr_l = t_l + (logB_r);
	SETDIGIT_L (tptr_l, DIGITS_L (t_l) - (logB_r));
	if (GE_L (tptr_l, n_l)) {
		sub_l (tptr_l, n_l, p_l);
	}
	else {
		cpy_l (p_l, tptr_l);
	}
}