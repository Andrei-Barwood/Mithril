#ifndef MITHRIL_CONTEXT_H
#define MITHRIL_CONTEXT_H

#include "mithril_api.h"
#include "mithril_capabilities.h"
#include "mithril_config.h"
#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

mithril_status mithril_context_set_config(mithril_context *ctx, const mithril_config *config);
mithril_status mithril_context_get_config(const mithril_context *ctx, mithril_config *out_config);
mithril_status mithril_context_get_capabilities(const mithril_context *ctx, mithril_capabilities *out_caps);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_CONTEXT_H */
