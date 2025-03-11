#include "flint.h"

int vcheck_l (CLINT n_l) {
	unsigned int error = E_VCHECK_OK;
	// encontrar el puntero a null y depurarlo
	if (n_l == NULL) {
		error = E_VCHECK_MEM;
	}
	else {
		// encontrar el overflow
		if (((unsigned int) DIGITS_L (n_l)) > CLINTMAXDIGIT) {
			error = E_VCHECK_OFL;
		}
		else {
			// encontrar ceros sobrantes
			if ((DIGITS_L (n_l) > 0) && (n_l[DIGITS_L (n_l)] == 0)) {
				error = E_VCHECK_LDZ;
			}
		}
	}
	return error;
}