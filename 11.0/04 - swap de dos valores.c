#include "flint.h"

#define SWAP(a, b) ((a) ^=(b), (b) ^=(a), (a)^=(b))
#define SWAP_L(a_l, b_l) \
				(xor_l((a_l), (b_l), (a_l)), \
				 xor_l((b_l), (a_l), (b_l)), \
				 xor_l((a_l), (b_l), (a_l)))