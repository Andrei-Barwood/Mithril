#include "provider_flint.h"

#include "mithril/mithril_provider.h"
#include "provider_flint_internal.h"

static mithril_provider_flint_state g_flint_state = {0};

static const mithril_provider_ops g_flint_ops = {
    .on_activate = mithril_provider_flint_on_activate,
    .on_deactivate = mithril_provider_flint_on_deactivate,
    .get_capabilities = mithril_provider_flint_get_capabilities
};

static const mithril_provider_crypto_ops g_flint_crypto_ops = {
    .bigint_add = mithril_provider_flint_bigint_add,
    .bigint_sub = mithril_provider_flint_bigint_sub,
    .bigint_mul = mithril_provider_flint_bigint_mul,
    .modarith_add_mod = mithril_provider_flint_modarith_add_mod,
    .modarith_mul_mod = mithril_provider_flint_modarith_mul_mod
};

mithril_status mithril_provider_flint_register(mithril_context *ctx) {
    mithril_provider_descriptor descriptor;

    descriptor.name = "flint";
    descriptor.abi_version = MITHRIL_PROVIDER_ABI_VERSION;
    descriptor.declared_capabilities = MITHRIL_CAP_BIGINT | MITHRIL_CAP_MODARITH;
    descriptor.ops = &g_flint_ops;
    descriptor.crypto_ops = &g_flint_crypto_ops;
    descriptor.user_data = &g_flint_state;

    return mithril_provider_register(ctx, &descriptor);
}
