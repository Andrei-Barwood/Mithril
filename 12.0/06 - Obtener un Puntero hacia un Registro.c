#include "flint.h"

clint * get_reg_l (unsigned int reg) {
    if (!registers.created || (reg >= registers.noofregs))
 {
     return (clint *) NULL;
 }
    return registers.reg_l[reg];
}