#include "mithril/mithril_version.h"

uint32_t mithril_version_major(void) {
    return MITHRIL_API_VERSION_MAJOR;
}

uint32_t mithril_version_minor(void) {
    return MITHRIL_API_VERSION_MINOR;
}

uint32_t mithril_version_patch(void) {
    return MITHRIL_API_VERSION_PATCH;
}

const char *mithril_version_string(void) {
    return "2.0.0";
}
