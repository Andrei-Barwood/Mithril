void xor_l (CLINT a_l, CLINT b_l, CLINT c_l) {
	CLINT d_l;
	clint *r_l, *s_l, *t_l;
	clint *msdptrr_l;
	clint *msdptrs_l;

	if (DIGITS_L (a_l) < DIGITS_L (b_l)) {
		r_l = LSDPTR_L (b_l);
		s_l = LSDPTR_L (a_l);
		msdptrr_l = MSDPTR_L (b_l);
		msdptrs_l = MSDPTR_L (a_l);
	}
	t_l = LSDPTR_L (d_l);
	SETDIGITS_L (d_l, DIGITS_L (r_l - 1));
	while (s_l <= msdptrs_l) {
		*t_l++ = *r_l++ ^ *s_l++;
	}
	while (r_l <= msdptrr_l) {
		*t_l++ = *r_l++;
	}
	cpy_l (c_l, d_l);
}