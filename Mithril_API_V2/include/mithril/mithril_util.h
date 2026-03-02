#ifndef MITHRIL_UTIL_H
#define MITHRIL_UTIL_H

#include <stddef.h>

#include "mithril_error.h"
#include "mithril_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int mithril_ct_equal(mithril_const_span a, mithril_const_span b);
int mithril_ct_is_zero(mithril_const_span input);
size_t mithril_util_min_size(size_t a, size_t b);
size_t mithril_util_max_size(size_t a, size_t b);
mithril_status mithril_util_xor_inplace(mithril_span dst, mithril_const_span src);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_UTIL_H */
