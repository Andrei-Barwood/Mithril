#include "flint.h"

int setbit_l (CLINT a_l, unsigned int pos) {
	int res = 0;
	unsigned int i;
	USHORT shorts = (USHORT)(pos >> LDBITPERDGT);
	USHORT bitpos = (USHORT)(p & (BITPERDGT - 1));
	USHORT m = 1U << bitpos;

	if (pos >= CLINTMAXBIT) {
		return E_CLINT_OFL;
	}

	if (shorts >= DIGITS_L (a_l)) {
		for (i = DIGITS_L (a_l) + 1; i <= shorts +1; i++) {
			a_l[i] = 0;
		}
		SETDIGITS_L (a_l, shorts + 1);
	}

	if (a_l[shorts + 1] & m) {
		res = 1;
	}
	a_l[shorts + 1] |= m;
	return res;
}