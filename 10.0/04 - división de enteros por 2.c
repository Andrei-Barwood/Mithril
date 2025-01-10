#include <flint.h>

int shr_l (CLINT a_l) {
	clint *ap_l;
	USHORT help, carry = 0;

	if (EQZ_L (a_l))
		return E_CLINT_UFL;

	for (ap_l = MSDPTR_L (a_l); ap_l > a_l; ap_l--) {
		help = (USHORT)((USHORT)(*ap_l >> 1) | (USHORT)(carry << (BITPERDGT - 1)));
		carry = (USHORT)(*ap_l & 1U);
		*ap_l = help;
	}
	RMLDZRS_L (a_l);
	return E_CLINT_OK;
}