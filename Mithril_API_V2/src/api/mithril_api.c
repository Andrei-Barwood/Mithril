#include "mithril/mithril_api.h"

#include <stdlib.h>

#include "../core/core_internal.h"
#include "../providers/provider_c23/provider_c23.h"
#include "../providers/provider_flint/provider_flint.h"
#include "../providers/provider_sodium/provider_sodium.h"

mithril_status mithril_init(mithril_context **out_ctx, const mithril_init_options *options) {
    mithril_context *ctx;
    mithril_status status;

    (void)options;

    if (out_ctx == NULL) {
        return MITHRIL_ERR_INVALID_ARGUMENT;
    }

    ctx = (mithril_context *)calloc(1u, sizeof(*ctx));
    if (ctx == NULL) {
        return MITHRIL_ERR_INTERNAL;
    }

    ctx->initialized = 1;
    mithril_core_config_default(&ctx->config);
    mithril_core_capabilities_default(&ctx->capabilities);
    mithril_core_registry_init(ctx);

    status = mithril_provider_sodium_register(ctx);
    if (status != MITHRIL_OK) {
        mithril_core_registry_shutdown(ctx);
        free(ctx);
        return status;
    }

    status = mithril_provider_activate(ctx, "sodium");
    if (status != MITHRIL_OK) {
        mithril_core_registry_shutdown(ctx);
        free(ctx);
        return status;
    }

    (void)mithril_provider_c23_register(ctx);
#if defined(MITHRIL_ENABLE_PROVIDER_FLINT)
    (void)mithril_provider_flint_register(ctx);
#endif

    *out_ctx = ctx;
    return MITHRIL_OK;
}

void mithril_shutdown(mithril_context *ctx) {
    if (ctx == NULL) {
        return;
    }

    ctx->initialized = 0;
    mithril_core_registry_shutdown(ctx);
    free(ctx);
}

int mithril_is_initialized(const mithril_context *ctx) {
    if (ctx == NULL) {
        return 0;
    }

    return (ctx->initialized == 1) ? 1 : 0;
}
