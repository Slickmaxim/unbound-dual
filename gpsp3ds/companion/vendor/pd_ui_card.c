// PokeDaisy trainer card renderer hosted with a single bounded workspace.
#include "pd_card.h"
#include "pd_font.h"
#include "pd_ui_internal.h"
#include <stdlib.h>
static uint32_t *pixels;
void cp_card_reset(void){free(pixels);pixels=NULL;}
void ui_card_tab(struct pd_ui*ui,struct pd_canvas*c,const struct pd_game*g,const struct pd_snapshot*s){
 struct pd_card_info info;
 if(!pd_card_info(g,s,&info)){ui_notice(c,"TRAINER CARD","Available after loading your save.",NULL);return;}
 if(!pixels)pixels=malloc(PD_CARD_W*PD_CARD_H*sizeof(*pixels));
 if(!pixels||!pd_card_render(g,&info,ui->cardBack,true,false,pixels)){
  ui_notice(c,"TRAINER CARD","Card graphics unavailable in this ROM.",NULL);return;
 }
 int l,t,r,b;pd_card_bounds(pixels,&l,&t,&r,&b);
 int w=r-l,h=b-t,x=(c->w-w)/2,y=(c->h-h-PD_FONT_HEIGHT-6)/2;
 if(y<0)y=0;
 for(int dy=0;dy<h;++dy)for(int dx=0;dx<w;++dx){uint32_t p=pixels[(t+dy)*PD_CARD_W+l+dx];
  if(p&&x+dx>=c->cx0&&x+dx<c->cx1&&y+dy>=c->cy0&&y+dy<c->cy1)c->px[(y+dy)*c->w+x+dx]=p&0xFFFFFF;}
 const char*caption=ui->cardBack?"TAP FOR FRONT":"TAP FOR BACK";
 pd_text(c,(c->w-pd_text_width(caption))/2,y+h+6,caption,PD_ON_BACKDROP,PD_ON_BACKDROP_SHADOW);
 ui_add_hit(ui,x,y,w,h,HIT_CARD);
}
bool ui_card_act(struct pd_ui*ui,int id){if(id!=HIT_CARD)return false;ui->cardBack=!ui->cardBack;return true;}
bool ui_card_tick(struct pd_ui*ui,unsigned ms){(void)ui;(void)ms;return false;}
