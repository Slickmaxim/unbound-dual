/* Unbound Dual fast-forward controls and compact top-screen badge. GPL-3.0. */
#ifndef UD_FAST_FORWARD_H
#define UD_FAST_FORWARD_H
#include <stdint.h>

/* Keep persisted values 1..3 compatible with gpSP: N extra frames = (N+1)x.
 * Legacy 0 (Max), missing settings and invalid values become 2x. */
static inline int ff_speed_normalize(int saved) {
   return saved < 1 ? 1 : saved > 3 ? 3 : saved;
}
static inline int ff_speed_cycle(int speed, int direction) {
   return 1 + (ff_speed_normalize(speed)-1 + (direction<0 ? 2 : 1)) % 3;
}
#define FF_BADGE_W 24
#define FF_BADGE_H 9
#define FF_BADGE_INK ((uint16_t)0xFFE8)
#define FF_BADGE_BG ((uint16_t)0x1082)

/* Small pixel lettering, generated directly rather than loading an asset.
 * Chevrons and digits use the same pixels on GPU and CPU output paths. */
static inline uint16_t ff_badge_pixel(int speed, int x, int y) {
   static const unsigned char glyphs[6][5] = {
      {4,2,1,2,4}, {7,1,7,4,7}, {7,1,7,1,7},
      {5,5,7,1,1}, {5,5,2,5,5}, {0,0,0,0,0}
   };
   if (x<0 || x>=FF_BADGE_W || y<2 || y>=7) return FF_BADGE_BG;
   int glyph=5, local=0;
   if (x>=2 && x<5) { glyph=0;local=x-2; }
   else if (x>=6 && x<9) { glyph=0;local=x-6; }
   else if (x>=13 && x<16) { glyph=ff_speed_normalize(speed);local=x-13; }
   else if (x>=18 && x<21) { glyph=4;local=x-18; }
   return glyphs[glyph][y-2] & (4>>local) ? FF_BADGE_INK : FF_BADGE_BG;
}

/* RGB565 3DS output: rotated columns, bottom row first. In 800px mode
 * duplicate columns to keep the badge's physical size unchanged.
 * On release, native/Fit black borders must also lose the old badge. */
static inline void ff_badge_blit(uint16_t* fb,int fw,int gx,int gy,int gw,int gh,int speed) {
   int scale=fw/400, x0=fw-(FF_BADGE_W+4)*scale;
   for (int y=0;y<FF_BADGE_H;y++)
      for (int x=0;x<FF_BADGE_W*scale;x++) {
         int sx=x0+x,sy=4+y;
         if (speed || sx<gx || sx>=gx+gw || sy<gy || sy>=gy+gh)
            fb[sx*240+239-sy]=speed ? ff_badge_pixel(speed,x/scale,y) : 0;
      }
}
#endif
