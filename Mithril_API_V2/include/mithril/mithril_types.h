#ifndef MITHRIL_TYPES_H
#define MITHRIL_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t mithril_byte;
typedef uint64_t mithril_u64;

typedef struct mithril_span {
    mithril_byte *ptr;
    size_t len;
} mithril_span;

typedef struct mithril_const_span {
    const mithril_byte *ptr;
    size_t len;
} mithril_const_span;

#ifdef __cplusplus
}
#endif

#endif /* MITHRIL_TYPES_H */
