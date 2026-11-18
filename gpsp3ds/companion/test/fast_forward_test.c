// User-visible speed compatibility and rotated 3DS badge pixels. GPL-3.0.
#include "../../source/fast_forward.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void check_badge(int fw,int gx,int gy,int gw,int gh) {
 size_t count=fw*240;
 uint16_t* guarded=malloc((count+2)*sizeof(*guarded));assert(guarded);
 uint16_t* original=malloc(count*sizeof(*original));assert(original);
 uint16_t* fb=guarded+1;guarded[0]=0x1234;guarded[count+1]=0x5678;
 // A known frame: black outside the game viewport, varied RGB565 inside.
 for(int x=0;x<fw;x++)for(int y=0;y<240;y++)
  fb[x*240+239-y]=x>=gx&&x<gx+gw&&y>=gy&&y<gy+gh ? (uint16_t)(1+(x+y)%65534) : 0;
 memcpy(original,fb,count*sizeof(*fb));
 int scale=fw/400,x0=fw-(FF_BADGE_W+4)*scale;
 for(int speed=1;speed<=3;speed++){
  ff_badge_blit(fb,fw,gx,gy,gw,gh,speed);
  for(int y=0;y<240;y++)for(int x=0;x<fw;x++){
   int at=x*240+239-y;
   uint16_t expected=x>=x0&&x<x0+FF_BADGE_W*scale&&y>=4&&y<4+FF_BADGE_H
      ? ff_badge_pixel(speed,(x-x0)/scale,y-4) : original[at];
   assert(fb[at]==expected);
  }
  // Next frame replaces game pixels; turning off must clear border pixels too.
  for(int x=gx;x<gx+gw;x++)for(int y=gy;y<gy+gh;y++)
   fb[x*240+239-y]=original[x*240+239-y];
  ff_badge_blit(fb,fw,gx,gy,gw,gh,0);
  assert(!memcmp(fb,original,count*sizeof(*fb)));
  assert(guarded[0]==0x1234&&guarded[count+1]==0x5678);
 }
 free(original);free(guarded);
}
int main(void){
 assert(ff_speed_normalize(0)==1&&ff_speed_normalize(-1)==1);
 for(int i=1;i<=3;i++)assert(ff_speed_normalize(i)==i);
 assert(ff_speed_normalize(99)==3);
 assert(ff_speed_cycle(1,-1)==3&&ff_speed_cycle(3,1)==1);
 assert(ff_speed_cycle(1,1)==2&&ff_speed_cycle(2,-1)==1);
 for(int i=1;i<=3;i++){
  int ink=0;for(int y=0;y<FF_BADGE_H;y++)for(int x=0;x<FF_BADGE_W;x++)ink+=ff_badge_pixel(i,x,y)==FF_BADGE_INK;
  assert(ink>0);
 }
 check_badge(400,80,40,240,160); // 1:1, including black borders
 check_badge(400,20,0,360,240); // Fit: badge straddles game and border
 check_badge(400,0,0,400,240); // Stretch
 check_badge(800,40,0,720,240); // wide Fit: physical size stays the same
 check_badge(800,0,0,800,240); // wide Stretch
 puts("Fast forward: legacy Max -> 2x, retained 2x/3x/4x, cycling, badge pixels/bounds/removal PASS");
}
