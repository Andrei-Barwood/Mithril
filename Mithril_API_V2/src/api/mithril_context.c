#include "mithril/mithril_context.h"

#include "../core/core_internal.h"

mithril_status mithril_context_set_config(mithril_context *ctx, const mithril_config *config) {
    mithril_status status;

    if (ctx == 0 || config == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    status = mithril_core_config_validate(config);
    if (status != MITHRIL_OK) {
        return status;
    }

    ctx->config = *config;
    return MITHRIL_OK;
}

mithril_status mithril_context_get_config(const mithril_context *ctx, mithril_config *out_config) {
    if (ctx == 0 || out_config == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    *out_config = ctx->config;
    return MITHRIL_OK;
}

mithril_status mithril_context_get_capabilities(const mithril_context *ctx, mithril_capabilities *out_caps) {
    if (ctx == 0 || out_caps == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    *out_caps = ctx->capabilities;
    return MITHRIL_OK;
}
