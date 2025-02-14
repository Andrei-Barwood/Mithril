#include "flint.h"

int testbit_l (CLINT a_l, unsigned int pos) {
	int res = 0;
	USHORT shorts = (USHORT)(pos >> LDBITPERDGT);
	USHORT bitpos = (USHORT)(pos & (BITPERDGT - 1))
	if (shorts < DIGITS_L (a_l)) {
		if (a_l[shorts + 1] & (USHORT)(1U << bitpos))
			res = 1;
	}
	return res;
}