#include "mithril/mithril_memory.h"

#include <stdlib.h>

mithril_status mithril_secure_alloc(mithril_span *out_buffer, size_t len) {
    if (out_buffer == NULL || len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    out_buffer->ptr = (mithril_byte *)calloc(len, sizeof(mithril_byte));
    if (out_buffer->ptr == NULL) {
        out_buffer->len = 0u;
        return MITHRIL_ERR_INTERNAL;
    }

    out_buffer->len = len;
    return MITHRIL_OK;
}

void mithril_secure_zero(void *ptr, size_t len) {
    volatile unsigned char *p;
    size_t i;

    if (ptr == NULL || len == 0u) {
        return;
    }

    p = (volatile unsigned char *)ptr;
    for (i = 0u; i < len; ++i) {
        p[i] = 0u;
    }
}

void mithril_secure_free(mithril_span *buffer) {
    if (buffer == NULL || buffer->ptr == NULL) {
        return;
    }

    mithril_secure_zero(buffer->ptr, buffer->len);
    free(buffer->ptr);
    buffer->ptr = NULL;
    buffer->len = 0u;
}

mithril_status mithril_secure_copy(mithril_span dst, mithril_const_span src) {
    size_t i;

    if (dst.ptr == NULL || src.ptr == NULL) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (dst.len < src.len) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    for (i = 0u; i < src.len; ++i) {
        dst.ptr[i] = src.ptr[i];
    }

    return MITHRIL_OK;
}
