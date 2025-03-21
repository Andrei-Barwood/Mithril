#include "flint.h"

int purgeall_reg_l (void) {
    unsigned i;
    if (registers.created) {
        for (i = 0; i < registers.noofregs; i++) {
            memset (registers.reg_l[i], 0, CLINTMAXBYTE);
        }
        return E_CLINT_OK;
    }
    return E_CLINT_NOR;
}

void free_reg_l (void) {
    if (registers.created == 1) {
        destroy_reg_l ();
    }
    if (registers.created) {
        --registers.created;
    }
}