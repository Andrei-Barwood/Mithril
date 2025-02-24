#include "flint.h"

static char ntable[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'a', 'b', 'c', 'd', 'e', 'f' };
char * xclint2str_l (CLINT n_l, USHORT base, int showbase) {
	CLINTD u_l, r_l;
	int i = 0;
	static char N[CLINTMAXBIT + 3];
	if (2U > base || base > 16U) {
		return (char *)NULL;
	}
	cpy_l (u_l, n_l);
	do {
		(void) udiv_l (u_l, base, u_l, r_l);
		if (EQZ_L (r_l)) {
			N[i++] = '0';
		}
		else {
			N[i++] = (char) ntable[*LSDPTR_L (r_l) & 0xff];
		}
	}
	while (GTZ_L (u_l));
	if (showbase) {
		switch (base) {
		case 2:
			N[i++] = 'b';
			N[i++] = '0';
			break;
		case 8:
			N[i++] = '0';
			break;
		case 16:
			N[i++] = 'x';
			N[i++] = '0';
			break;
		}
	}
	N[i] = '0';
	return strrev_l (N);
}