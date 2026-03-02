#include "core_internal.h"

void mithril_core_capabilities_default(mithril_capabilities *out_caps) {
    if (out_caps == 0) {
        return;
    }

    out_caps->feature_flags = 0u;
    out_caps->provider_count = 0u;
    out_caps->reserved = 0u;
}

void mithril_capabilities_default(mithril_capabilities *out_caps) {
    mithril_core_capabilities_default(out_caps);
}

int mithril_capabilities_has(const mithril_capabilities *caps, uint64_t capability_flag) {
    if (caps == 0) {
        return 0;
    }

    return ((caps->feature_flags & capability_flag) != 0u) ? 1 : 0;
}
