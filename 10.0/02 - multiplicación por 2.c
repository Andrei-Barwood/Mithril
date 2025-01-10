#include <flint.h>

int shl_l (CLINT a_l) {
	clint *ap_l, *msdptra_l;
	ULONG carry = 0L;
	int error = E_CLINT_OK;

	RMLDZRS_L (a_l);

	if (ld_l (a_l) >= (USHORT)CLINTMAXBIT) {
		SETDIGITS_L (a_l, CLINTMAXDIGIT);
		error = E_CLINT_OFL;
	}
	msdptra_l = MSDPTR_L (a_l);
	for (*ap_l = LSDPTR_L (a_l); ap_l <= msdptra_l; ap_l++) {
		*ap_l = (USHORT)(carry = ((ULONG) *ap_l << 1) | (carry >> BITPERDGT));
	}
	if (carry >> BITPERDGT) {
		if (DIGITS_L (a_l) < CLINTMAXDIGIT) {
			*ap_l = 1;
			SETDIGITS_L (a_l, DIGITS_L (a_l) + 1);
			error = E_CLINT_OK;
		}
		else {
			error = E_CLINT_OFL;
		}
	}
	RMLDZRS_L (a_l);
	return error;
}