#include "mithril/mithril_util.h"

#include "core_internal.h"

int mithril_ct_equal(mithril_const_span a, mithril_const_span b) {
    if (a.ptr == NULL || b.ptr == NULL) {
        return 0;
    }

    if (a.len != b.len) {
        return 0;
    }

    return mithril_core_ct_compare(a.ptr, b.ptr, a.len);
}

int mithril_ct_is_zero(mithril_const_span input) {
    if (input.ptr == NULL) {
        return 0;
    }

    return mithril_core_ct_all_zero(input.ptr, input.len);
}

size_t mithril_util_min_size(size_t a, size_t b) {
    return (a < b) ? a : b;
}

size_t mithril_util_max_size(size_t a, size_t b) {
    return (a > b) ? a : b;
}

mithril_status mithril_util_xor_inplace(mithril_span dst, mithril_const_span src) {
    size_t i;

    if (dst.ptr == NULL || src.ptr == NULL) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (dst.len != src.len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < dst.len; ++i) {
        dst.ptr[i] ^= src.ptr[i];
    }

    return MITHRIL_OK;
}
