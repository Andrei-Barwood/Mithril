#ifndef MITHRIL_MEMORY_H
#define MITHRIL_MEMORY_H

#include <stddef.h>

#include "mithril_error.h"
#include "mithril_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mithril_status mithril_secure_alloc(mithril_span *out_buffer, size_t len);
void mithril_secure_free(mithril_span *buffer);
void mithril_secure_zero(void *ptr, size_t len);
mithril_status mithril_secure_copy(mithril_span dst, mithril_const_span src);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_MEMORY_H */
