#ifndef MITHRIL_ERROR_H
#define MITHRIL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mithril_status {
    MITHRIL_OK = 0,
    MITHRIL_ERR_INVALID_ARGUMENT = 1,
    MITHRIL_ERR_NOT_INITIALIZED = 2,
    MITHRIL_ERR_ALREADY_INITIALIZED = 3,
    MITHRIL_ERR_NO_PROVIDER = 4,
    MITHRIL_ERR_NOT_IMPLEMENTED = 5,
    MITHRIL_ERR_INTERNAL = 255
} mithril_status;

const char *mithril_status_string(mithril_status status);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_ERROR_H */
