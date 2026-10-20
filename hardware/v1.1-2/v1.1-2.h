// subtraction_pli.h
// PLI Module for Secure Subtraction Operations
// Mithril_Cryptography_API - Icarus Verilog Interface

#ifndef SUBTRACTION_PLI_H
#define SUBTRACTION_PLI_H

#include <vpi_user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Return codes for PLI functions
#define E_FLINT_OK 0
#define E_FLINT_UFL 1 // Underflow

// PLI function declarations
PLI_INT32 secure_subtraction_calltf(PLI_BYTE8 *user_data);
PLI_INT32 secure_subtraction_compiletf(PLI_BYTE8 *user_data);
PLI_INT32 secure_subtraction_sizetf(PLI_BYTE8 *user_data);

// Registration function
void register_subtraction_pli(void);

#endif
