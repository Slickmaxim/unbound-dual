// Active Bag scrolling cadence, independent of slow RAM snapshot polling.
// GPL-3.0. No catch-up redraws; expensive frames reduce the target rate.
#ifndef CP_SCROLL_REFRESH_H
#define CP_SCROLL_REFRESH_H
#include <stdint.h>
#include <stdbool.h>
struct cp_scroll_refresh { uint64_t next_us; unsigned draw_us; bool drawn; };
static inline unsigned cp_scroll_period_us(unsigned cost_us) {
    if (cost_us<=3000) return 33333; // up to 30 Hz
    if (cost_us<=5000) return 41667; // about 24 Hz
    // Above 5 ms, back off toward ~12.5% of one core, capped at 250 ms.
    if (cost_us>31250) return 250000;
    unsigned period=cost_us*8;
    return period<50000 ? 50000 : period;
}
static inline bool cp_scroll_due(const struct cp_scroll_refresh* r,uint64_t now_us) {
    return !r->drawn || now_us>=r->next_us;
}
static inline void cp_scroll_feedback(struct cp_scroll_refresh* r,uint64_t now_us,unsigned draw_us) {
    r->draw_us=r->drawn ? (unsigned)(((uint64_t)r->draw_us*3+draw_us)/4) : draw_us;
    unsigned period=cp_scroll_period_us(r->draw_us);
    // Preserve the fractional cadence (24 Hz on a 60 Hz loop alternates
    // 2/3 frames). Discard long stalls instead of creating a catch-up burst.
    r->next_us=r->drawn&&now_us>=r->next_us&&now_us-r->next_us<period
        ? r->next_us+period : now_us+period;
    r->drawn=true;
}
#endif
