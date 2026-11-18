// Check interaction cadence against the actual 60 Hz emulator loop. GPL-3.0.
#include "../scroll_refresh.h"
#include <assert.h>
#include <stdio.h>
static int redraws(unsigned cost){
 struct cp_scroll_refresh r={0};int count=0;
 for(uint64_t t=0;t<1000000;t+=16667)if(cp_scroll_due(&r,t)){
  cp_scroll_feedback(&r,t,cost);count++;
 }
 return count;
}
int main(void){
 int normal=redraws(2000),busy=redraws(4000),expensive=redraws(10000);
 assert(normal>=29&&normal<=31);assert(busy>=23&&busy<=25);
 assert(expensive<busy&&expensive<=14);
 struct cp_scroll_refresh r={0};assert(cp_scroll_due(&r,0));
 cp_scroll_feedback(&r,0,2000);assert(!cp_scroll_due(&r,10000));
 assert(cp_scroll_due(&r,500000));cp_scroll_feedback(&r,500000,2000);
 assert(!cp_scroll_due(&r,500000));assert(r.next_us>500000);
 // Feedback recovers from slow frames, rather than permanently locking
 // the interaction cadence to the reduced RAM polling rate.
 for(int i=0;i<30;i++)cp_scroll_feedback(&r,r.next_us,1000);
 assert(cp_scroll_period_us(r.draw_us)==33333);
 printf("Bag scheduler: 60 Hz loop -> %d/%d/%d redraws, bounded expensive work, no catch-up, recovery PASS\n",normal,busy,expensive);
}
