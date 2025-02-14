#include "flint.h"

int clearbit_l (CLINT a_l, unsigned int pos) {
	int res = 0;
	USHORT shorts = (USHORT)(pos >> LDBITPERDGT);
	USHORT bitpos = (USHORT)(pos & (BITPERDGT - 1));
	USHORT m = 1U << bitpos;

	if (shorts < DIGITS_L (a_l)) {
		if (a_l[shorts + 1] & m) {
			res = 1;
		}
		a_l[shorts + 1] &= (USHORT)(~m);
		RMLDZRS_L (a_l);
	}
	return res;
}