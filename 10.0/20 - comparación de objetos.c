#include "flint.h"

int cmp_l (CLINT a_l, CLINT b_l) {
	clint *msdptra_l, *msdptrb_l;
	int la = DIGITS_L (a_l);
	int lb = DIGITS_L (b_l);

	if (la == 0 && lb == 0) {
		return 0;
	}
	while (a_l[la] == 0 && la > 0) {
		--la;
	}
	while (b_l[lb] == 0 && lb > 0) {
		--lb;
	}
	if (la == 0 && lb == 0) {
		return 0;
	}
	if (la > lb) {
		return 1;
	}
	if (la < lb) {
		return -1;
	}

	msdptra_l = a_l + la;
	msdptrb_l = b_l + lb;

	while ((*msdptra_l == *msdptrb_l) && (msdptra_l > a_l)) {
		msdptra_l--;
		msdptrb_l--;
	}

	if (msdptra_l == a_l) {
		return 0;
	}
	if (*msdptra_l > *msdptrb_l) {
		return 1;
	}
	else {
		return -1;
	}
}