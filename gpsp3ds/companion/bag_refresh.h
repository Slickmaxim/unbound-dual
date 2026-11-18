// Fixed Bag presentation timer for Unbound Dual. GPL-3.0.
#ifndef CP_BAG_REFRESH_H
#define CP_BAG_REFRESH_H
#include <stdbool.h>
#include <stdint.h>
#define CP_BAG_PERIOD_US 33333u
struct cp_bag_refresh {
    bool active;
    uint64_t next_us,work_us;
    unsigned frames;
};
// Called even when the view is clean and there is no touch input.
static inline bool cp_bag_refresh_due(struct cp_bag_refresh* r,bool open,uint64_t now_us) {
    if (!open) { r->active=false;return false; }
    if (!r->active) *r=(struct cp_bag_refresh){.active=true,.next_us=now_us};
    return now_us>=r->next_us;
}
static inline void cp_bag_refresh_presented(struct cp_bag_refresh* r,uint64_t now_us,unsigned work_us) {
    // Keep the 30 Hz phase, but discard missed frames after menu/sleep/stalls.
    // Drawing cost is recorded for profiling; it never reduces the target.
    r->next_us=now_us>=r->next_us&&now_us-r->next_us<CP_BAG_PERIOD_US
        ? r->next_us+CP_BAG_PERIOD_US : now_us+CP_BAG_PERIOD_US;
    r->work_us+=work_us;r->frames++;
}
#endif
