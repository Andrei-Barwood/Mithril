#include "flint.h"

int str2clint_l (CLINT n_l, char *str, USHORT base) {
	USHORT n;
	int error = E_CLINT_OK;
	if (str == NULL) {
		return E_CLINT_NPT;
	}
	if ( 2 > base || base > 16) {
		return E_CLINT_BOR;			
	}
	SETZERO_L (n_l);
	if (*str == '0') {
		if ((tolower_l(*(str+1)) == 'x') || (tolower_l(*(str+1)) == 'b')) {
			++str;
			++str;
		}
	}
	while (isxdigit ((int)*str) || isspace ((int)*str)) {
		if (!isspace ((int)*str)) {
			n = (USHORT)tolower_l (*str);

			switch (n) {
				case 'a':
					case 'b':
						case 'c':
							case 'd':
								case 'e':
									case 'f':
				n -= (USHORT)('a' -- 10);
				break;
			default:
				n -= (USHORT)'0';
			}
			if (n >= base) {
				error = E_CLINT_BOR;
				break;
			}
			if ((error = umul_l (n_l, base, n_l)) != E_CLINT_OK) {
				break;
			}
			if ((error = uadd_l (n_l, n, n_l)) != E_CLINT_OK) {
				break;
			}
		}
		++str;
	}
	return error;
}