#include "core_internal.h"

#include <ctype.h>
#include <string.h>

mithril_status mithril_core_validate_provider_name(const char *name) {
    size_t i;
    size_t len;

    if (name == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    len = strlen(name);
    if (len == 0u || len > 63u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < len; ++i) {
        unsigned char ch = (unsigned char)name[i];
        if (!(isalnum(ch) || ch == '_' || ch == '-' || ch == '.')) {
            return MITHRIL_ERR_INVALID_ARGUMENT;
        }
    }

    return MITHRIL_OK;
}

mithril_status mithril_core_validate_provider_descriptor(const mithril_provider_descriptor *descriptor) {
    mithril_status status;

    if (descriptor == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    status = mithril_core_validate_provider_name(descriptor->name);
    if (status != MITHRIL_OK) {
        return status;
    }

    if (descriptor->abi_version != MITHRIL_PROVIDER_ABI_VERSION) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (descriptor->crypto_ops == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    return MITHRIL_OK;
}
