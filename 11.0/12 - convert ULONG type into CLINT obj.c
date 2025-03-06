#include "flint.h"

void u2clint_l (CLINT num_l, CLINT u_l) {
	*LSDPTR_L (num_l) = (USHORT)(ul & 0xffff);
	*(LSDPTR_L (num_l) + 1) = (USHORT(ul >> 16) & 0xffff);
	SETDIGITS_L (num_l, 2);
	RMLDZRS_L (num_l);
}