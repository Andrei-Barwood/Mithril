#include <flint.h>

int mexp2_l (CLINT a_l, USHORT k, CLINT p_l, CLINT m_l) {
	CLINT tmp_l;
	if (EQZ_L (m_l)) {
		return E_CLINT_DBZ;
	}

	if (k > 0) {
		cpy_l (tmp_l, a_l);
		while (k-- > 0) {
			msqr_l (tmp_l, tmp_l, m_l);
		}
		cpy_l (p_l, tmp_l);
	}
	else {
		mod_l (a_l, m_l, p_l);
	}
	return E_CLINT_OK;
}