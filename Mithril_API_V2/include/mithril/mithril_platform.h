#ifndef MITHRIL_PLATFORM_H
#define MITHRIL_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

mithril_status mithril_platform_random(uint8_t *out, size_t out_len);
uint64_t mithril_platform_monotonic_ns(void);
void mithril_platform_secure_clear(void *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_PLATFORM_H */
