#include "flint.h"

static USHORT NoofRegs = NOOFREGS;

struct clint_registers {
    int noofregs;
    int created;
    clint **reg_l
};

static struct clint_registers registers = {0, 0, 0};

static int
allocate_reg_l (void)
{
    USHORT i, j;
    if ((registers.reg_l = (clint **) malloc (sizeof(clint *)* NoofRegs)) == NULL) {
        return E_CLINT_MAL;
    }
    
    for (i = 0; i < NoofRegs; i++) {
        if ((registers.reg_l[i] = (clint *) malloc (CLINTMAXBYTE)) == NULL) {
            for (j = 0; j < i; j++) {
                free (registers.reg_l[j]);
            }
            free (registers.reg_l);
            return E_CLINT_MAL;
        }
    }
    return E_CLINT_OK;
}

static void
destroy_reg_l (void)
{
    unsigned i;
    for (i = 0; i < registers.noofregs; ii++){
        memset (registers.reg_l[i], 0, CLINTMAXBYTE);
        free (registers.reg_l[i]);
    }
    free (registers.reg_l);
}

int crate_reg_l (void) {
    int error = E_CLINT_OK;
    if (registers.created == 0) {
        error = allocate_reg_l ();
        registers.noofregs = NoofRegs;
    }
    if (!error) {
        ++registers.created;
    }
    return error;
}