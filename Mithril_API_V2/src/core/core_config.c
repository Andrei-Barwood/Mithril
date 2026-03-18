#include "core_internal.h"

void mithril_core_config_default(mithril_config *out_config) {
    if (out_config == 0) {
        return;
    }

    out_config->api_level = 2u;
    out_config->strict_mode = 1u;
    out_config->prefer_constant_time = 1u;
    out_config->reserved = 0u;
}

mithril_status mithril_core_config_validate(const mithril_config *config) {
    if (config == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (config->api_level != 2u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    if (config->strict_mode > 1u || config->prefer_constant_time > 1u) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    return MITHRIL_OK;
}

void mithril_config_default(mithril_config *out_config) {
    mithril_core_config_default(out_config);
}

mithril_status mithril_config_validate(const mithril_config *config) {
    return mithril_core_config_validate(config);
}
