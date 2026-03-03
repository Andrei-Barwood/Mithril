#include "mithril/mithril_error.h"

const char *mithril_status_string(mithril_status status) {
    switch (status) {
        case MITHRIL_OK:
            return "MITHRIL_OK";
        case MITHRIL_ERR_INVALID_ARGUMENT:
            return "MITHRIL_ERR_INVALID_ARGUMENT";
        case MITHRIL_ERR_NOT_INITIALIZED:
            return "MITHRIL_ERR_NOT_INITIALIZED";
        case MITHRIL_ERR_ALREADY_INITIALIZED:
            return "MITHRIL_ERR_ALREADY_INITIALIZED";
        case MITHRIL_ERR_NO_PROVIDER:
            return "MITHRIL_ERR_NO_PROVIDER";
        case MITHRIL_ERR_NOT_IMPLEMENTED:
            return "MITHRIL_ERR_NOT_IMPLEMENTED";
        case MITHRIL_ERR_AEAD_AUTH_FAILED:
            return "MITHRIL_ERR_AEAD_AUTH_FAILED";
        case MITHRIL_ERR_SIGNATURE_INVALID:
            return "MITHRIL_ERR_SIGNATURE_INVALID";
        case MITHRIL_ERR_INTERNAL:
            return "MITHRIL_ERR_INTERNAL";
        default:
            return "MITHRIL_ERR_UNKNOWN";
    }
}
