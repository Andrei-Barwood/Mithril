#ifndef MITHRIL_CORE_INTERNAL_H
#define MITHRIL_CORE_INTERNAL_H

#include <stddef.h>
#include <stdint.h>

#include "mithril/mithril_api.h"
#include "mithril/mithril_capabilities.h"
#include "mithril/mithril_config.h"
#include "mithril/mithril_provider.h"

#define MITHRIL_CORE_MAX_PROVIDERS 16u

typedef struct mithril_provider_entry {
    int in_use;
    mithril_provider_descriptor descriptor;
    mithril_capabilities capabilities;
} mithril_provider_entry;

struct mithril_context {
    int initialized;
    mithril_config config;
    mithril_capabilities capabilities;
    mithril_provider_entry providers[MITHRIL_CORE_MAX_PROVIDERS];
    uint32_t provider_count;
    int active_provider_index;
};

void mithril_core_config_default(mithril_config *out_config);
mithril_status mithril_core_config_validate(const mithril_config *config);
void mithril_core_capabilities_default(mithril_capabilities *out_caps);

mithril_status mithril_core_validate_provider_name(const char *name);
mithril_status mithril_core_validate_provider_descriptor(const mithril_provider_descriptor *descriptor);

void mithril_core_registry_init(mithril_context *ctx);
void mithril_core_registry_shutdown(mithril_context *ctx);
int mithril_core_find_provider_index(const mithril_context *ctx, const char *name);
const mithril_provider_entry *mithril_core_get_active_provider(const mithril_context *ctx);

int mithril_core_ct_compare(const uint8_t *a, const uint8_t *b, size_t len);
int mithril_core_ct_all_zero(const uint8_t *data, size_t len);

#endif /* MITHRIL_CORE_INTERNAL_H */
