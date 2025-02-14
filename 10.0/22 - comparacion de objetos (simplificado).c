#include "flint.h"

int equ_l (CLINT a_l, CLINT b_l) {
	clint *msdptra_l, *msdptrb_l;
	int la = DIGITS_L (a_l);
	int lb = DIGITS_L (b_l);
	if (la == 0 && lb == 0) {
		return 1;
	}
	while (a_l[la] == 0 && la > 0) {
		--la;
	}
	while (b_l[lb] == 0 && lb > 0) {
		--lb;
	}
	if (la == 0 && lb == 0) {
		return 1;
	}
	if (la != lb) {
		return 0;
	}
	msdptra_l = a_l + la;
	msdptrb_l = b_l + lb;

	while ((*msdptra_l == *msdptrb_l) && (msdptra_l > a_l)) {
		msdptra_l--;
		msdptrb_l--;
	}
	return (msdptra_l > a_l ? 0 : 1);
}