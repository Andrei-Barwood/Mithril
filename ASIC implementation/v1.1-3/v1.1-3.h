//
// mixed_addition_pli.h
// Mithril_Cryptography_API PLI Module
//
// Created by Andres Barbudo on 28-08-25.
//

#ifndef MIXED_ADDITION_PLI_H
#define MIXED_ADDITION_PLI_H

#include "vpi_user.h"
#include "flint.h"
#include "fmpz.h"

#ifdef __cplusplus
extern "C" {
#endif

// PLI system function prototypes
PLI_INT32 mixed_addition_compiletf(PLI_BYTE8* user_data);
PLI_INT32 mixed_addition_calltf(PLI_BYTE8* user_data);

// Registration function
void mixed_addition_register(void);

#ifdef __cplusplus
}
#endif

#endif
