#include "mithril/mithril_platform.h"

#if defined(__APPLE__)
#include <Security/SecRandom.h>
#endif

mithril_status mithril_platform_random(uint8_t *out, size_t out_len) {
    if (out == 0 || out_len == 0u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

#if defined(__APPLE__)
    if (SecRandomCopyBytes(kSecRandomDefault, out_len, out) != errSecSuccess) {
        return MITHRIL_ERR_INTERNAL;
    }
    return MITHRIL_OK;
#else
    (void)out;
    (void)out_len;
    return MITHRIL_ERR_NOT_IMPLEMENTED;
#endif
}
