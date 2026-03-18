#ifndef MITHRIL_VERSION_H
#define MITHRIL_VERSION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MITHRIL_API_VERSION_MAJOR 2u
#define MITHRIL_API_VERSION_MINOR 0u
#define MITHRIL_API_VERSION_PATCH 0u

uint32_t mithril_version_major(void);
uint32_t mithril_version_minor(void);
uint32_t mithril_version_patch(void);
const char *mithril_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_VERSION_H */
