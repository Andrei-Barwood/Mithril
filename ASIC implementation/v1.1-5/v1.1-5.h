//
// increment_pli.h
// Mithril_Cryptography_API PLI Module
//
// Created for Icarus Verilog PLI interface
//

#ifndef INCREMENT_PLI_H
#define INCREMENT_PLI_H

#include <vpi_user.h>
#include <fmpz.h>

/* PLI function prototypes */
PLI_INT32 fmpz_inc_compiletf(PLI_BYTE8 *user_data);
PLI_INT32 fmpz_inc_calltf(PLI_BYTE8 *user_data);

PLI_INT32 fmpz_inc_inplace_compiletf(PLI_BYTE8 *user_data);
PLI_INT32 fmpz_inc_inplace_calltf(PLI_BYTE8 *user_data);

PLI_INT32 fmpz_inc_mod_compiletf(PLI_BYTE8 *user_data);
PLI_INT32 fmpz_inc_mod_calltf(PLI_BYTE8 *user_data);

/* Registration function */
void register_increment_pli(void);

/* Error codes */
#define E_FMPZ_OK 0
#define E_FMPZ_OFL 1

#endif
