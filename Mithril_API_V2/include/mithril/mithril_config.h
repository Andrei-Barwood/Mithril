#ifndef MITHRIL_CONFIG_H
#define MITHRIL_CONFIG_H

#include <stdint.h>

#include "mithril_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mithril_config {
    uint32_t api_level;
    uint32_t strict_mode;
    uint32_t prefer_constant_time;
    uint32_t reserved;
} mithril_config;

void mithril_config_default(mithril_config *out_config);
mithril_status mithril_config_validate(const mithril_config *config);

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_CONFIG_H */
