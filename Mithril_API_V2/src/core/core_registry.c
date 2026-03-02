#include "core_internal.h"

#include <string.h>

void mithril_core_registry_init(mithril_context *ctx) {
    uint32_t i;

    if (ctx == 0) {
        return;
    }

    for (i = 0u; i < MITHRIL_CORE_MAX_PROVIDERS; ++i) {
        ctx->providers[i].in_use = 0;
        ctx->providers[i].descriptor.name = 0;
        ctx->providers[i].descriptor.abi_version = 0u;
        ctx->providers[i].descriptor.declared_capabilities = 0u;
        ctx->providers[i].descriptor.ops = 0;
        ctx->providers[i].descriptor.crypto_ops = 0;
        ctx->providers[i].descriptor.user_data = 0;
        mithril_core_capabilities_default(&ctx->providers[i].capabilities);
    }

    ctx->provider_count = 0u;
    ctx->active_provider_index = -1;
}

void mithril_core_registry_shutdown(mithril_context *ctx) {
    uint32_t i;

    if (ctx == 0) {
        return;
    }

    if (ctx->active_provider_index >= 0 &&
        (uint32_t)ctx->active_provider_index < MITHRIL_CORE_MAX_PROVIDERS) {
        mithril_provider_entry *active = &ctx->providers[(uint32_t)ctx->active_provider_index];
        if (active->in_use == 1 && active->descriptor.ops != 0 && active->descriptor.ops->on_deactivate != 0) {
            active->descriptor.ops->on_deactivate(active->descriptor.user_data);
        }
    }

    for (i = 0u; i < MITHRIL_CORE_MAX_PROVIDERS; ++i) {
        ctx->providers[i].in_use = 0;
    }

    ctx->provider_count = 0u;
    ctx->active_provider_index = -1;
    mithril_core_capabilities_default(&ctx->capabilities);
}

int mithril_core_find_provider_index(const mithril_context *ctx, const char *name) {
    uint32_t i;

    if (ctx == 0 || name == 0) {
        return -1;
    }

    for (i = 0u; i < MITHRIL_CORE_MAX_PROVIDERS; ++i) {
        if (ctx->providers[i].in_use == 1 && ctx->providers[i].descriptor.name != 0) {
            if (strcmp(ctx->providers[i].descriptor.name, name) == 0) {
                return (int)i;
            }
        }
    }

    return -1;
}

mithril_status mithril_provider_register(mithril_context *ctx, const mithril_provider_descriptor *descriptor) {
    uint32_t slot;
    mithril_status status;
    mithril_capabilities caps;

    if (ctx == 0 || descriptor == 0) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    status = mithril_core_validate_provider_descriptor(descriptor);
    if (status != MITHRIL_OK) {
        return status;
    }

    if (mithril_core_find_provider_index(ctx, descriptor->name) >= 0) {
        return MITHRIL_ERR_ALREADY_INITIALIZED;
    }

    if (ctx->provider_count >= MITHRIL_CORE_MAX_PROVIDERS) {
        return MITHRIL_ERR_INTERNAL;
    }

    slot = 0u;
    while (slot < MITHRIL_CORE_MAX_PROVIDERS && ctx->providers[slot].in_use == 1) {
        ++slot;
    }
    if (slot >= MITHRIL_CORE_MAX_PROVIDERS) {
        return MITHRIL_ERR_INTERNAL;
    }

    ctx->providers[slot].in_use = 1;
    ctx->providers[slot].descriptor = *descriptor;

    mithril_core_capabilities_default(&caps);
    caps.feature_flags = descriptor->declared_capabilities;
    caps.provider_count = 1u;

    if (descriptor->ops != 0 && descriptor->ops->get_capabilities != 0) {
        status = descriptor->ops->get_capabilities(descriptor->user_data, &caps);
        if (status != MITHRIL_OK) {
            ctx->providers[slot].in_use = 0;
            return status;
        }
    }

    ctx->providers[slot].capabilities = caps;
    ctx->provider_count += 1u;
    ctx->capabilities.provider_count = ctx->provider_count;

    return MITHRIL_OK;
}

uint32_t mithril_provider_count(const mithril_context *ctx) {
    if (ctx == 0) {
        return 0u;
    }
    return ctx->provider_count;
}
