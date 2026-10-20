//
// mixed_mult_pli.h
// Mithril_Cryptography_API - PLI Interface
//
// Created by Andres Barbudo on 03-09-25.
//

#ifndef MIXED_MULT_PLI_H
#define MIXED_MULT_PLI_H

#include <vpi_user.h>
#include "flint.h"
#include "fmpz.h"

/* PLI Error codes */
#define PLI_SUCCESS 0
#define PLI_ERROR -1
#define PLI_OVERFLOW 1

/* PLI function prototypes */
PLI_INT32 mixed_mult_register(void);
PLI_INT32 mixed_mult_call(PLI_BYTE8 *user_data);
PLI_INT32 mixed_mult_compile(PLI_BYTE8 *user_data);

/* Internal helper functions */
static void convert_verilog_to_fmpz(vpiHandle arg, fmpz_t result);
static void convert_fmpz_to_verilog(fmpz_t src, vpiHandle result);
static PLI_UINT32 get_verilog_uint(vpiHandle arg);

#endif
