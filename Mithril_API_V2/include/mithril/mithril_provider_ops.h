#ifndef MITHRIL_PROVIDER_OPS_H
#define MITHRIL_PROVIDER_OPS_H

#include "mithril_capabilities.h"
#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mithril_provider_ops {
    mithril_status (*on_activate)(void *user_data);
    void (*on_deactivate)(void *user_data);
    mithril_status (*get_capabilities)(void *user_data, mithril_capabilities *out_caps);
} mithril_provider_ops;

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_PROVIDER_OPS_H */
