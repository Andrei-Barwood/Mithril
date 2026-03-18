#include "core_internal.h"

int mithril_core_ct_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t diff;
    size_t i;

    if (a == 0 || b == 0) {
        return 0;
    }

    diff = 0u;
    for (i = 0u; i < len; ++i) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }

    return (diff == 0u) ? 1 : 0;
}

int mithril_core_ct_all_zero(const uint8_t *data, size_t len) {
    uint8_t acc;
    size_t i;

    if (data == 0) {
        return 0;
    }

    acc = 0u;
    for (i = 0u; i < len; ++i) {
        acc |= data[i];
    }

    return (acc == 0u) ? 1 : 0;
}
