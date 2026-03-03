#include "core_internal.h"

mithril_status mithril_provider_activate(mithril_context *ctx, const char *name) {
    int index;
    mithril_provider_entry *target;
    mithril_status status;

    if (ctx == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    status = mithril_core_validate_provider_name(name);
    if (status != MITHRIL_OK) {
        return status;
    }

    index = mithril_core_find_provider_index(ctx, name);
    if (index < 0) {
        return MITHRIL_ERR_NO_PROVIDER;
    }

    if (ctx->active_provider_index == index) {
        return MITHRIL_OK;
    }

    target = &ctx->providers[(uint32_t)index];

    if (target->descriptor.ops != 0 && target->descriptor.ops->on_activate != 0) {
        status = target->descriptor.ops->on_activate(target->descriptor.user_data);
        status = mithril_core_normalize_lifecycle_status(status);
        if (status != MITHRIL_OK) {
            return status;
        }
    }

    if (ctx->active_provider_index >= 0 && (uint32_t)ctx->active_provider_index < MITHRIL_CORE_MAX_PROVIDERS) {
        mithril_provider_entry *previous = &ctx->providers[(uint32_t)ctx->active_provider_index];
        if (previous->in_use == 1 && previous->descriptor.ops != 0 && previous->descriptor.ops->on_deactivate != 0) {
            previous->descriptor.ops->on_deactivate(previous->descriptor.user_data);
        }
    }

    ctx->active_provider_index = index;
    ctx->capabilities = target->capabilities;
    ctx->capabilities.provider_count = ctx->provider_count;

    return MITHRIL_OK;
}

const char *mithril_provider_active_name(const mithril_context *ctx) {
    if (ctx == 0 || ctx->active_provider_index < 0) {
        return 0;
    }

    if ((uint32_t)ctx->active_provider_index >= MITHRIL_CORE_MAX_PROVIDERS) {
        return 0;
    }

    if (ctx->providers[(uint32_t)ctx->active_provider_index].in_use != 1) {
        return 0;
    }

    return ctx->providers[(uint32_t)ctx->active_provider_index].descriptor.name;
}

const mithril_provider_entry *mithril_core_get_active_provider(const mithril_context *ctx) {
    if (ctx == 0 || ctx->active_provider_index < 0) {
        return 0;
    }

    if ((uint32_t)ctx->active_provider_index >= MITHRIL_CORE_MAX_PROVIDERS) {
        return 0;
    }

    if (ctx->providers[(uint32_t)ctx->active_provider_index].in_use != 1) {
        return 0;
    }

    return &ctx->providers[(uint32_t)ctx->active_provider_index];
}

mithril_status mithril_provider_get_capabilities(const mithril_context *ctx, mithril_capabilities *out_caps) {
    if (ctx == 0 || out_caps == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    *out_caps = ctx->capabilities;
    return MITHRIL_OK;
}
