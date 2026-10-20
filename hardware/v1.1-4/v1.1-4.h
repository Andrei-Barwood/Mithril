//
// mixed_subtraction_pli.h
// Mithril_Cryptography_PLI_Module
//
// Created by Andres Barbudo on 21-08-25.
//

#ifndef MIXED_SUBTRACTION_PLI_H
#define MIXED_SUBTRACTION_PLI_H

#include <vpi_user.h>
#include <fmpz.h>

// PLI function prototypes
PLI_INT32 mixed_subtraction_compiletf(PLI_BYTE8* user_data);
PLI_INT32 mixed_subtraction_calltf(PLI_BYTE8* user_data);
PLI_INT32 mixed_subtraction_sizetf(PLI_BYTE8* user_data);

// Registration function
void mixed_subtraction_register(void);

// Internal helper functions
int convert_verilog_to_fmpz(vpiHandle handle, fmpz_t result);
int convert_fmpz_to_verilog(fmpz_t value, vpiHandle handle);

#endif
