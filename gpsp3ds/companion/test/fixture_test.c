// End-to-end decoder + read-only UI fixtures; test-only state initialization
// avoids pretending a copyrighted ROM is present. No production hash bypass.
#include "../vendor/pd_game.c"
#include "../companion.c"
#include "../readonly_ui.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned char ew[262144], iw[32768],saved_ew[262144],saved_iw[32768];
static uint16_t output[320*240];
static bool fixture_read(void*ctx,uint32_t a,void*out,size_t n){
 (void)ctx;if(a>=0x02000000&&a<=0x02040000&&n<=0x02040000-a){memcpy(out,ew+a-0x02000000,n);return true;}
 if(a>=0x03000000&&a<=0x03008000&&n<=0x03008000-a){memcpy(out,iw+a-0x03000000,n);return true;}return false;
}
static void load_bytes(const char*root,const char*fixture,const char*part,void*out,size_t n){char p[1024];snprintf(p,sizeof(p),"%s/%s/%s.bin",root,fixture,part);FILE*f=fopen(p,"rb");assert(f);assert(fread(out,1,n,f)==n);fclose(f);}
static void tap_id(int id){
 for(int i=0;i<S->ui.hitCount;++i)if(S->ui.hits[i].id==id){int x=S->ui.hits[i].x+S->ui.hits[i].w/2,y=S->ui.hits[i].y+S->ui.hits[i].h/2;companion_touch(0,x,y);assert(!companion_touch(2,x,y));return;}
 fprintf(stderr,"missing hit %d\n",id);abort();
}
static void image(const char*root,const char*name){char path[1024];snprintf(path,sizeof(path),"%s/%s.ppm",root,name);FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n320 240\n255\n");for(int i=0;i<320*240;++i){uint16_t c=output[i];unsigned char b[3]={(c>>11)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};fwrite(b,1,3,f);}fclose(f);}
int main(int argc,char**argv){assert(argc==3);const char*fixtures[]={"unbound","unbound_battle"};
 for(int f=0;f<2;++f){
  load_bytes(argv[1],fixtures[f],"ewram",ew,sizeof(ew));load_bytes(argv[1],fixtures[f],"iwram",iw,sizeof(iw));memcpy(saved_ew,ew,sizeof(ew));memcpy(saved_iw,iw,sizeof(iw));
  companion_close();S=cp_calloc(1,sizeof(*S));assert(S);
  S->game.kind=PD_GAME_FIRERED;S->game.cfg=&PD_CONFIG_UNBOUND;S->game.guide=PD_GUIDE_UNBOUND;S->game.language='E';S->game.shinyBelow=16;S->game.loadId=f+1;
  strcpy(S->game.code,"BPRE");strcpy(S->game.sha1,"b4776b82a4c7915d0fadeaa27e013523f99dfd94");S->game.entry=pd_game_entry_for(S->game.sha1);assert(S->game.entry);overlay(&S->game,S->game.entry->fieldCfg);
  pd_settings_defaults(&S->settings);pd_look_apply(&S->game,&S->settings);cp_ui_init(&S->ui,&S->settings);S->read=fixture_read;S->interval=200;
#ifdef COMPANION_NATIVE
  S->detail=-1;
#endif
  supported=true;S->dirty=true;
  assert(companion_update(1));assert(S->snap.partyCount==1);assert(S->snap.party[0].species==496);assert(S->snap.party[0].level==11);assert(S->snap.party[0].hp==34);
  companion_draw(output);if(!f)image(argv[2],"party");assert(!companion_update(2));
  // Regression: a running VBlank counter and unrelated save data must
  // not redraw the party, but changes to HP/moves must remain visible.
  struct pd_snapshot changed=S->snap;
  ++changed.frame;++changed.playMinutes;++changed.gameStats[0];changed.flags[0]^=1;
  assert(!visible_changed(&S->snap,&changed,PD_TAB_PARTY));
  assert(!visible_changed(&S->snap,&changed,PD_TAB_BAG));
  assert(!visible_changed(&S->snap,&changed,PD_TAB_MAP));
  --changed.party[0].hp;assert(visible_changed(&S->snap,&changed,PD_TAB_PARTY));
  assert(!visible_changed(&S->snap,&changed,PD_TAB_BAG));
  changed=S->snap;changed.bag[0].items[0].quantity^=1;assert(visible_changed(&S->snap,&changed,PD_TAB_BAG));
  changed=S->snap;++changed.x;assert(visible_changed(&S->snap,&changed,PD_TAB_MAP));
  tap_id(HIT_SLOT);assert(S->ui.summarySlot==0);companion_draw(output);if(!f)image(argv[2],"summary");
  for(int i=0;i<S->ui.hitCount;++i){int id=S->ui.hits[i].id;assert(id!=HIT_BAG_USE&&id!=HIT_BAG_GIVE&&id!=HIT_SUMMARY_TAKE&&id!=HIT_SUMMARY_PC&&id!=HIT_MAP_FLY&&id!=HIT_MAP_TAKE_OFF);}
  tap_id(9001);companion_draw(output);if(!f)image(argv[2],"bag");assert(S->ui.tab==PD_TAB_BAG);assert(companion_bag_open());
#ifdef COMPANION_NATIVE
  if(S->snap.bag[0].count){tap_id(9200);companion_draw(output);if(!f)image(argv[2],"item");tap_id(9104);companion_draw(output);}
#endif
  tap_id(9002);companion_draw(output);if(!f)image(argv[2],"map-no-rom");assert(S->ui.tab==PD_TAB_MAP);assert(!companion_bag_open());
  tap_id(9003);assert(S->ui.tab==PD_TAB_CARD);companion_draw(output);
  struct pd_card_info card;assert(pd_card_info(&S->game,&S->snap,&card));
  assert(card.style==PD_CARD_UNBOUND);assert(card.trainerId==S->snap.trainerId);
  assert(ui_card_act(&S->ui,HIT_CARD)&&S->ui.cardBack);
  companion_options(true,false);assert(S->settings.darkMode&&pd_dark&&!cp_bottom_smooth);
  companion_draw(output);if(!f)image(argv[2],"card-no-rom");
  tap_id(9000);companion_draw(output);uint16_t light[320*240];memcpy(light,output,sizeof(light));
  companion_options(false,true);companion_draw(output);assert(!S->settings.darkMode&&!pd_dark&&cp_bottom_smooth);assert(memcmp(light,output,sizeof(light)));
  if(!f)image(argv[2],"party-light");
  companion_options(true,false);companion_draw(output);if(!f)image(argv[2],"party-dark");
  companion_touch(0,288,224);assert(companion_touch(2,288,224));
  companion_feedback(4001);assert(companion_interval_ms()==500);companion_feedback(4001);assert(companion_interval_ms()==1000);
  assert(companion_memory()<2u*1024u*1024u);
  assert(!memcmp(ew,saved_ew,sizeof(ew)));assert(!memcmp(iw,saved_iw,sizeof(iw)));
  printf("%s: party=%d HP=%d/%d, read-only touch/menu PASS, memory=%zu\n",fixtures[f],S->snap.partyCount,S->snap.party[0].hp,S->snap.party[0].maxHp,companion_memory());
  companion_close();assert(cp_memory_used()==0);
 }
 return 0;
}
