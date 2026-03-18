#ifndef MITHRIL_CAPABILITIES_H
#define MITHRIL_CAPABILITIES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MITHRIL_CAP_RNG = (1ULL << 0),
    MITHRIL_CAP_HASH = (1ULL << 1),
    MITHRIL_CAP_AEAD = (1ULL << 2),
    MITHRIL_CAP_KEX = (1ULL << 3),
    MITHRIL_CAP_SIGN = (1ULL << 4),
    MITHRIL_CAP_BIGINT = (1ULL << 5),
    MITHRIL_CAP_MODARITH = (1ULL << 6)
};

typedef struct mithril_capabilities {
    uint64_t feature_flags;
    uint32_t provider_count;
    uint32_t reserved;
} mithril_capabilities;

void mithril_capabilities_default(mithril_capabilities *out_caps);
int mithril_capabilities_has(const mithril_capabilities *caps, uint64_t capability_flag);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_CAPABILITIES_H */
