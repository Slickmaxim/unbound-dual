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
static void check_bag_partial_draw(void) {
 struct pd_snapshot saved=S->snap;
 memset(S->snap.bag,0,sizeof(S->snap.bag));S->snap.bagOk=true;
 S->snap.bag[PD_POCKET_ITEMS].count=80;
 for(int i=0;i<80;i++)S->snap.bag[PD_POCKET_ITEMS].items[i]=(struct pd_item){i+1,i+1};
 static uint16_t partial[320*240];
 for(int details=0;details<2;details++){
  S->ui.bagSelected=details?0:-1;S->ui.bagScroll=0;
  companion_draw(output);
  int x=0,y=0;
  for(int i=0;i<S->ui.hitCount;i++)if(S->ui.hits[i].id==HIT_BAG_LIST){
   x=S->ui.hits[i].x+30;y=S->ui.hits[i].y+S->ui.hits[i].h-20;break;
  }
  assert(x>0&&y>40);
  companion_touch(0,x,y);companion_draw(output);
  companion_touch(1,x,y-29);assert(S->ui.bagScroll>0);
  int rect[4];assert(companion_draw_bag_scroll(output,rect));
  assert(rect[2]<320&&rect[3]<208);
  memcpy(partial,output,sizeof(partial));
  companion_draw(output);assert(!memcmp(partial,output,sizeof(partial)));
  companion_touch(1,x,y-29);assert(!companion_draw_bag_scroll(output,rect));
  companion_touch(2,x,y-29);
 }
 S->snap=saved;S->ui.bagSelected=-1;S->ui.bagScroll=0;
 puts("Bag: partial scroll matches full rendering with and without item details");
}
static bool item_fixture_rom(void*ctx,uint32_t a,void*out,size_t n){
 (void)ctx;
 uint32_t base=S->game.cfg->itemDescs->base;
 if(n!=32||a!=base+13*44)return false;
 unsigned char record[32]={0};
 record[14]=13;record[28]=0xe1;record[29]=0x16;record[30]=0x0a;record[31]=0x08;
 memcpy(out,record,sizeof(record));
 return true;
}
static bool guest_ready(void){return true;}
static bool guest_committed;
static int guest_calls;
static bool item_guest_call(uint32_t address,const uint32_t args[4],uint32_t*result,bool query){
 const struct pd_config*cfg=S->game.cfg;
 guest_calls++;
 if(address==cfg->fieldLocked){assert(query);*result=0;}
 else if(address==cfg->fieldCheckBag){assert(query&&args[0]==13&&args[1]==1);*result=1;}
 else if(address==cfg->fieldRemoveBag){assert(!query&&args[0]==13&&args[1]==1);*result=1;}
 else {assert(args[0]==cfg->playerParty&&args[1]==13);*result=0;}
 return true;
}
static void guest_finish(bool commit){guest_committed=commit;}
static void check_bag_item_touch(void){
 const struct companion_guest fake={guest_ready,guest_ready,item_guest_call,guest_finish};
 companion_item_bind(&fake);
 S->game.romRead=item_fixture_rom;S->item_profile=true;
 S->ui.bagSelected=0;companion_draw(output);
 tap_id(HIT_BAG_USE);assert(S->ui.itemPicking);
 companion_draw(output);tap_id(HIT_ITEM_CANCEL);
 assert(!S->ui.itemPicking&&!S->item_request.frames);
 companion_draw(output);tap_id(HIT_BAG_USE);companion_draw(output);
 tap_id(HIT_ITEM_SLOT);
 assert(S->item_request.frames&&guest_calls==0);
 companion_item_tick();
 assert(!S->item_request.frames&&guest_committed&&guest_calls==5);
 companion_item_tick();assert(guest_calls==5);
 companion_item_cancel();S->ui.bagSelected=-1;S->game.romRead=NULL;
 companion_item_bind(NULL);
 puts("Bag: touch, cancel, single queued application and commit passed");
}
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
  check_bag_partial_draw();
  if(!f)check_bag_item_touch();
  companion_draw(output);
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
