// Exercise the actual PokeDaisy card scaler with high-contrast synthetic art.
#include "../vendor/pd_ui_party.c"
#include <assert.h>
#include <stdio.h>
extern int cp_bottom_smooth;
int main(void){
 uint32_t src[4]={0,0xFFFFFF,0xFFFFFF,0},dst[9];
 struct pd_canvas canvas;pd_canvas_init(&canvas,3,3,dst);
 struct card_scale scale={3,2,0};
 cp_bottom_smooth=0;screen_up(&canvas,src,2,2,&scale);
 for(int i=0;i<9;++i)assert(dst[i]==0||dst[i]==0xFFFFFF);
 assert(dst[0]!=dst[2]&&dst[0]!=dst[6]);
 cp_bottom_smooth=1;screen_up(&canvas,src,2,2,&scale);
 bool mixed=false;for(int i=0;i<9;++i)if(dst[i]!=0&&dst[i]!=0xFFFFFF)mixed=true;
 assert(mixed);puts("Bottom filtering: nearest preserves palette, smooth blends PASS");
}
