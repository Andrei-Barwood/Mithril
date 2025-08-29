#ifndef FLINT_PLI_H
#define FLINT_PLI_H

#include "veriuser.h"
#include "acc_user.h"
// Note: FLINT headers would need to be available in your build environment
#include "flint.h"
#include "fmpz.h"

// PLI function prototypes
int pli_big_add_calltf(int user_data, int reason);
int pli_big_add_checktf(int user_data, int reason);
int pli_big_add_sizetf(int user_data, int reason);

#endif
