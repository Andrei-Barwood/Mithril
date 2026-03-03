#include "core_internal.h"

static int mithril_core_is_known_status(mithril_status status) {
    switch (status) {
        case MITHRIL_OK:
        case MITHRIL_ERR_INVALID_ARGUMENT:
        case MITHRIL_ERR_NOT_INITIALIZED:
        case MITHRIL_ERR_ALREADY_INITIALIZED:
        case MITHRIL_ERR_NO_PROVIDER:
        case MITHRIL_ERR_NOT_IMPLEMENTED:
        case MITHRIL_ERR_AEAD_AUTH_FAILED:
        case MITHRIL_ERR_SIGNATURE_INVALID:
        case MITHRIL_ERR_INTERNAL:
            return 1;
        default:
            return 0;
    }
}

mithril_status mithril_core_normalize_status(mithril_status status) {
    if (mithril_core_is_known_status(status) == 0) {
        return MITHRIL_ERR_INTERNAL;
    }

    return status;
}

mithril_status mithril_core_normalize_lifecycle_status(mithril_status status) {
    status = mithril_core_normalize_status(status);

    switch (status) {
        case MITHRIL_ERR_AEAD_AUTH_FAILED:
        case MITHRIL_ERR_SIGNATURE_INVALID:
            return MITHRIL_ERR_INTERNAL;
        default:
            return status;
    }
}
