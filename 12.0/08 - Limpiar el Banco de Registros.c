#include "flint.h"

int purge_reg_l (unsigned int reg) {
    if (!registers.created || (reg >= registers.noofregs)) {
        return E_CLINT_NOR;
    } 
    memset (registers.reg_l[reg], 0, CLINTMAXBYTE);
    return E_CLINT_OK;
}