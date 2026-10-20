// karats_pli.h
// PLI module for Karatsuba multiplication in IoT secure environments

#ifndef KARATS_PLI_H
#define KARATS_PLI_H

#include <vpi_user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// VPI system function declarations
PLI_INT32 karats_mul_calltf(PLI_BYTE8* user_data);
PLI_INT32 karats_mul_compiletf(PLI_BYTE8* user_data);
PLI_INT32 karats_mul_sizetf(PLI_BYTE8* user_data);

// Registration function
void register_karats_systfs(void);

#endif /* KARATS_PLI_H */
