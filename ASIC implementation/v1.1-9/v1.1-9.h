//
// secure_math_pli.h  
// Secure Mathematical Operations PLI Module for Icarus Verilog
// taken from Mithril_Cryptography_API
//

#ifndef SECURE_MATH_PLI_H
#define SECURE_MATH_PLI_H

#include <vpi_user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// VPI system task prototypes
PLI_INT32 secure_square_calltf(PLI_BYTE8* user_data);
PLI_INT32 secure_square_sizetf(PLI_BYTE8* user_data);
PLI_INT32 secure_square_compiletf(PLI_BYTE8* user_data);

// Registration function
void register_secure_math_tasks();

#ifdef __cplusplus
}
#endif

#endif /* SECURE_MATH_PLI_H */
