#include "mithril/mithril_platform.h"

#if defined(__APPLE__)
#include <mach/mach_time.h>
#endif

uint64_t mithril_platform_monotonic_ns(void) {
#if defined(__APPLE__)
    static mach_timebase_info_data_t timebase = {0, 0};
    uint64_t t;

    if (timebase.denom == 0u) {
        (void)mach_timebase_info(&timebase);
    }

    t = mach_absolute_time();
    return (uint64_t)((__uint128_t)t * (__uint128_t)timebase.numer / (__uint128_t)timebase.denom);
#else
    return 0u;
#endif
}
