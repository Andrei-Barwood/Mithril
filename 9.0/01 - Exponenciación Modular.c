#include "flint.h"

static int eventab[] = {0,0,1,0,1,0,1,0,3,0,1,0,1,0,1,0,6,0,1,0,12,0,1,0,3,0,1,0,3,0,1,0,12, ...};
static USHORT oddtab[] = {0,1,1,9,1,18,3,36,1,9,6,18,3,8,9,18,1,18,9,18,36,24,12,24,9,36,72, ...};

int mexpk_l (CLINT bas_l, CLINT exp_l, CLINT p_l, CLINT m_l) {
	CLINT a_l, a2_l;
	clint e_l[CLINTMAXSHORT + 1];
	CLINTD acc_l;
	clint **aptr_l, *ptr_l;
	int noofdigits, s, t, i;
	ULONG k;
	unsigned int lge, bit, digit, fk, word, pow4k, k_mask;

	if (EQZ_L(m_l)) {
		return E_CLINT_DBZ;
	}
	if (EQONE_L(m_l)) {
		SETZERO_L(p_l);					/* modulus = 1 ==> residue = 0 */
		return E_CLINT_OK;
	}

	cpy_l (a_l, bas_l);
	cpy_l (e_l, exp_l);

	if (EQZ_L (e_l)) {
		SETONE_L (p_l);
		return E_CLINT_OK;
	}
	if (EQZ_L (a_l)) {
		SETZERO_L (p_l);
		return E_CLINT_OK;
	}

	lge = ld_l (e_l);
	k = 8;
	while (k > 1 && ((k - 1) * (k << ((k - 1) << 1))/((1 << k) - k - 1)) >= lge - 1) {
		--k;
	}
	pow4k = 1U << k;
	k_mask = pow4k - 1U;

	if ((aptr_l = (clint **) malloc (sizeof(clint *) * pow4k)) == NULL) {
		return E_CLINT_MAL;
	}

	mod_l (a_l, m_l, a_l);
	aptr_l[1] = a_l;

	if (k > 1) {
		if ((ptr_l = (clint *) malloc (sizeof(CLINT) * ((pow4k >> 1) - 1))) == NULL) {
			return E_CLINT_MAL;
		}
		aptr_l[2] = a2_l;
		for (aptr_l[3] = ptr_l, i = 5; i < (int)pow4k; i+=2) {
			aptr_l[i] = aptr_l[i - 2] + CLINTMAXSHORT;
		}

		msqr_l (a_l, aptr_l[2], m_l);
		for (i = 3; i < (int)pow4k; i += 2) {
			mmul_l (aptr_l[2], aptr_l[i - 2], aptr_l[i], m_l);
		}
	}

	*(MSDPTR_L (e_l) + 1) = 0;

	noofdigits = (lge - 1)/k;
	fk = noofdigits * k;

	word = fk >> LDBITPERDGT;
	bit = fk & (BITPERDGT-1U);

	switch (k) {
	case 1:
	case 2:
	case 3:
	case 4:
		digit = ((ULONG)(e_l [word + 1]) >> bit) & k_mask;
		break;
	default:
		digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & k_mask;
	}

	if (digit != 0)   /* k-digit > 0 */
	{
		cpy_l (acc_l, aptr_l[oddtab[digit]]);
		t = eventab[digit];
		for (; y > 0; t--) {
			msqr_l (acc_l, acc_l, m_l);
		}
	}
	else 			 /* k-digit == 0 */
	{
		SETONE_L (acc_l);
	}

	for (--noofdigits, fk -= k; noofdigits >= 0; noofdigits--, fk -= k) {
		word = fk >> LDBITPERDGT;
		bit = fk & (BITPERDGT - 1U);

		switch (k) {
		case 1:
		case 2:
		case 3:
		case 4:
			digit = ((ULONG)(e_l[word + 1]) >> bit) & k_mask;
			break;
		default:
			digit = ((ULONG)(e_l[word + 1] | ((ULONG)e_l[word + 2] << BITPERDGT)) >> bit) & k_mask;
		}

		if (digit != 0) {
			t = eventab[digit];

			for (s = k - t; s > 0; s--) {
				msqr_l (acc_l, acc_l, m_l);
			}
			mmul_l (acc_l, aptr_l[oddtab[digit]], acc_l, m_l);

			for (; t > 0; t--) {
				msqr_l (acc_l, acc_l, m_l);
			}
		}
		else /* k-digit == 0 */
		{
			for (s = k; s > 0; s--) {
				msqr_l (acc_l, acc_l, m_l);
			}
		}
	}

	cpy_l (p_l, acc_l);

	free (aptr_l);
	if (ptr_l != NULL)		free (ptr_l);
	return E_CLINT_OK;

}