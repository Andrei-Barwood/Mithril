#ifndef MITHRIL_API_H
#define MITHRIL_API_H

#include "mithril_error.h"
#include "mithril_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mithril_context mithril_context;

typedef struct mithril_init_options {
    uint32_t reserved;
} mithril_init_options;

mithril_status mithril_init(mithril_context **out_ctx, const mithril_init_options *options);
void mithril_shutdown(mithril_context *ctx);
int mithril_is_initialized(const mithril_context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_API_H */
