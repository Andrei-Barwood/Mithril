//
// 
// PLI Interface Header
//

#ifndef MITHRIL_PLI_LIBSODIUM_H
#define MITHRIL_PLI_LIBSODIUM_H

#include "vpi_user.h"
#include <sodium.h>

// PLI callback function prototypes
PLI_INT32 mithril_add_scalar_calltf(PLI_BYTE8 *user_data);
PLI_INT32 mithril_add_mod_curve25519_calltf(PLI_BYTE8 *user_data);

// Registration function
void register_mithril_pli_tasks(void);

#endif // MITHRIL_PLI_LIBSODIUM_H
