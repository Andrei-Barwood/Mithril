#include "flint.h"

void free_l (CLINT reg_l) {
    if (NULL != reg_l) {
        memset (reg_l, 0, CLINTMAXBYTE);
        free (n_l);
    }
}