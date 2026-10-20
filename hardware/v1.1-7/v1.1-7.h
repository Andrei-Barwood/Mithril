//
// multiplication_pli.h
// Mithril_Cryptography_API PLI Interface
//
// Created for IoT secure multiplication operations
//

#ifndef MULTIPLICATION_PLI_H
#define MULTIPLICATION_PLI_H

#include <veriuser.h>
#include <acc_user.h>
#include <flint.h>
#include <fmpz.h>
#include "13_multiplication.h"

// PLI system function declarations
PLI_INT32 secure_mul_calltf(PLI_BYTE8 *user_data);
PLI_INT32 secure_mul_checktf(PLI_BYTE8 *user_data);
PLI_INT32 secure_mul_sizetf(PLI_BYTE8 *user_data);

// Registration function
void multiplication_pli_register(void);

#endif
