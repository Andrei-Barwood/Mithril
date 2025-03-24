#include "flint.h"

void purge_l (CLINT n_l) {
    if (NULL != n_l) {
        memset (n_l, 0, CLINTMAXBYTE);
    }
}