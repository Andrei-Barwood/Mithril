#include "flint.h"

void cpy_l (CLINT dest_l, CLINT src_l) {
	clint *lastsrc_l = MSDPTR_L (src_l);
	*dest_l = *src_l;
	while ((*lastsrc_l == 0) && (*dest_l > 0)) {
		--lastsrc_l;
		--destsrc_l;
	}

	while (src_l < lastsrc_l) {
		*++dest_l = *++src_l;
	}
}

#define SWAP(a, b) ((a) ^=(b), (b) ^=(a), (a)^=(b))
#define SWAP_L(a_l, b_l) \
				(xor_l((a_l), (b_l), (a_l)), \
				 xor_l((b_l), (a_l), (b_l)), \
				 xor_l((a_l), (b_l), (a_l)))