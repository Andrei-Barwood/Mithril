#include "mithril/mithril_platform.h"

void mithril_platform_secure_clear(void *ptr, size_t len) {
    volatile unsigned char *p;
    size_t i;

    if (ptr == 0 || len == 0u) {
        return;
    }

    p = (volatile unsigned char *)ptr;
    for (i = 0u; i < len; ++i) {
        p[i] = 0u;
    }
}
