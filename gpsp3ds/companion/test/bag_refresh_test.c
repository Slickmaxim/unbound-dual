// Fixed cadence must not depend on touch/dirty state or render-cost feedback.
#include "../bag_refresh.h"
#include <assert.h>
#include <stdio.h>
static int frames(unsigned cost){
 struct cp_bag_refresh r={0};
 for(uint64_t t=0;t<1000000;t+=16667)
  if(cp_bag_refresh_due(&r,true,t))cp_bag_refresh_presented(&r,t,cost);
 return r.frames;
}
int main(void){
 assert(frames(1000)==30);assert(frames(9000)==30);assert(frames(30000)==30);
 struct cp_bag_refresh r={0};assert(!cp_bag_refresh_due(&r,false,0));
 assert(cp_bag_refresh_due(&r,true,100));cp_bag_refresh_presented(&r,100,9000);
 assert(!cp_bag_refresh_due(&r,true,1000));
 assert(cp_bag_refresh_due(&r,true,100+CP_BAG_PERIOD_US));
 cp_bag_refresh_presented(&r,100+CP_BAG_PERIOD_US,9000);
 assert(!cp_bag_refresh_due(&r,false,34000));
 assert(cp_bag_refresh_due(&r,true,35000)); // reopening paints immediately
 assert(r.frames==0);cp_bag_refresh_presented(&r,35000,9000);
 assert(cp_bag_refresh_due(&r,true,1000000)); // long pause: no catch-up burst
 cp_bag_refresh_presented(&r,1000000,9000);
 assert(!cp_bag_refresh_due(&r,true,1000000));assert(r.next_us>1000000);
 puts("Bag fixed timer: 30 Hz for pending redraws, independent of cost feedback, reopen/stall PASS");
}
