#include "flint.h"

int shift_l (CLINT n_l, long int noofbits) {
	USHORT shorts = (USHORT)((ULONG)(noofbits < 0 ? -noofbits : noofbits) / BITPERDGT);
	USHORT bits = (USHORT)((ULONG)(noofbits < 0 ? -noofbits : noofbits) % BITPERDGT);
	long int resl;
	USHORT i;
	int error = E_CLINT_OK;
	clint *nptr_l;
	clint *msdptrn_l;
	RMLDZRS_L (n_l);
	resl = (int) ld_l (n_l) + noofbits;

	if (*n_l == 0) {
		return ((resl < 0) ? E_CLINT_UFL : E_CLINT_OK);
	}
	if (noofbits == 0) {
		return E_CLINT_OK;
	}
	if ((resl < 0) || (resl > (long) CLINTMAXBIT)) {
		error = ((resl < 0) ? E_CLINT_UFL : E_CLINT_OFL);
	}
	msdptrn_l = MSDPTR_L (n_l);
	if (noofbits < 0) {
		shorts = MIN (DIGITS_L (n_l), shorts);
		msdptrn_l = MSDPTR_L (n_l) - shorts;
		for (nptr_l = LSDPTR_L (n_l); nptr_l <= msdptrn_l; nptr_l++) {
			*nptr_l = *(nptr_l + shorts);
		}
		SETDIGITS_L (n_l, DIGITS_L (n_l) - (USHORT)shorts);
		for (i = 0; i < bits; i++) {
			shr_l (n_l);
		}
	}
	else {
		if (shorts < CLINTMAXDIGIT) {
			SETDIGITS_L (n_l, MIN (DIGITS_L (n_l) + shorts, CLINTMAXDIGIT));
			nptr_l = n_l + DIGITS_L (n_l);
			msdptrn_l = n_l + shorts;
			while (nptr_l > msdptrn_l) {
				*nptr_l = *(nptr_l - shorts);
				--nptr_l;
			} while (nptr_l > n_l) {
				*nptr_l-- = 0;
			}
			RMLDZRS_L (n_l);
			for (i = 0; i < bits; i++) {
				shl_l (n_l);
				}
			}
			else {
			SETZERO_L (n_l);
		}
	}
	return error;
}