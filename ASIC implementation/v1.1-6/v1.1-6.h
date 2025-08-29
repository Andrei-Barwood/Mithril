//
// fmpz_decrement_pli.h
// PLI Module for Icarus Verilog - Secure Decrement
//
// Created by Andres Barbudo on 21-08-25.
//

#ifndef FMPZ_DECREMENT_PLI_H
#define FMPZ_DECREMENT_PLI_H

#include <vpi_user.h>
#include <stdlib.h>
#include <stdio.h>

// Error codes for consistency with original design
typedef enum {
    E_FMPZ_OK = 0,
    E_FMPZ_UFL = -1  // Underflow error
} fmpz_error_t;

// PLI function declarations
PLI_INT32 secure_decrement_compiletf(PLI_BYTE8 *user_data);
PLI_INT32 secure_decrement_calltf(PLI_BYTE8 *user_data);
PLI_INT32 secure_decrement_sizetf(PLI_BYTE8 *user_data);

// Internal function
fmpz_error_t secure_dec(PLI_INT32 *value, PLI_INT32 width);

#endif
