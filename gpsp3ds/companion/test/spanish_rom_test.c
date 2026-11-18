// Optional integration check using a user's ROM plus upstream RAM fixtures.
// ROMs, snapshots and rendered outputs are never bundled with this test.
#include "../companion.c"
#include "../adapter.h"
#include "../localization.h"
#include "../vendor/pd_tables.h"
#include "../vendor/pd_font.h"
#include <assert.h>
#include <stdio.h>
unsigned char ewram[512*1024],iwram[64*1024];
unsigned char *memory_map_read[8192];
static uint16_t frame[320*240], english_frame[320*240];
static void ram(const char*root,const char*fixture,const char*name,void*out,size_t len){
 char path[1200];snprintf(path,sizeof(path),"%s/%s/%s.bin",root,fixture,name);
 FILE*f=fopen(path,"rb");assert(f&&fread(out,1,len,f)==len);fclose(f);
}
static void screenshot(const char*dir,const char*name){
 char path[1200];snprintf(path,sizeof(path),"%s/%s.ppm",dir,name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n320 240\n255\n");
 for(int i=0;i<320*240;i++){unsigned c=frame[i];unsigned char rgb[3]={((c>>11)&31)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);
}
int main(int argc,char**argv){
 assert(argc==4);
 // Read baseline RAM with the verified common addresses, while all ROM art,
 // descriptions and lookup tables are taken from the selected Spanish ROM.
 ram(argv[2],"unbound","ewram",ewram,262144);ram(argv[2],"unbound","iwram",iwram+32768,32768);
 companion_cache_directory(argv[3]);
 assert(cp_adapter_open(argv[1]));assert(companion_supported());
 // Model resident emulator ROM pages, without involving paging or the JIT.
 FILE* rf=fopen(argv[1],"rb");assert(rf);unsigned char* image=malloc(32u*1024u*1024u);assert(image);assert(fread(image,1,32u*1024u*1024u,rf)==32u*1024u*1024u);fclose(rf);
 for(unsigned i=0;i<1024;i++)memory_map_read[4096+i]=image+i*32768;
 assert(S&&S->game.language=='S');
 const struct pd_unbound_es_profile* profile=pd_unbound_es_profile(S->game.sha1);assert(profile);
 bool za=!strcmp(S->game.sha1,"d5b312465c0a95bcb6428bd61cc170994a2a65fc");
 bool legacy=!strcmp(S->game.sha1,"1104054a3061b35b85b04757fbca5a3686cca60c");
 assert(S->game.cfg->speciesCount==(za?1314:1294));
 assert(S->game.cfg->text->itemsCount==(za?767:729));
 assert(!pd_unbound_es_profile("117449bd7822fad0db293a4f7e364787e4f8543b"));
 // Compare adapter addresses with pointers actually referenced by game code.
 uint32_t ptr;memcpy(&ptr,image+0x1bc,4);assert(S->game.cfg->speciesInfo==ptr);
 memcpy(&ptr,image+0x1c8,4);assert(S->game.cfg->itemDescs->base==ptr);
 memcpy(&ptr,image+0x128,4);assert(S->game.cfg->frontPics==ptr);
 memcpy(&ptr,image+0x130,4);assert(S->game.cfg->palettes==ptr);
 memcpy(&ptr,image+0x138,4);assert(S->game.cfg->monIconTable==ptr);
 memcpy(&ptr,image+0x4323c,4);assert(S->game.cfg->speciesToNational==ptr);
 memcpy(&ptr,image+0x3ea7c,4);assert(S->game.cfg->learnsets==ptr);
 memcpy(&ptr,image+0x9899c,4);assert(S->game.cfg->itemIconTable==ptr);
 assert(!strcmp(S->game.cfg->text->natures[0],"Fuerte"));
 assert(!strcmp(S->game.cfg->text->mapsecs[0x59],legacy?"Ciudad Bellin":"Pueblo Bellin"));
 assert(!strcmp(pd_item_name(&S->game,710),legacy?"Esp. Oxidada":"Espada Oxidada"));
 assert(!strcmp(pd_move_name(&S->game,900,(char[80]){0},80),legacy?"Gigarrosión":"Gigacorros."));
 const struct pd_map_art* map=pd_map_art(&S->game);assert(map&&map->ok&&map->pages>0);
 char name[80];assert(!strcmp(pd_move_name(&S->game,33,name,sizeof(name)),"Placaje"));
 assert(!strcmp(pd_move_name(&S->game,45,name,sizeof(name)),"Gruñido"));
 assert(!strcmp(pd_item_name(&S->game,13),"Poción"));
 if(za){
  assert(!strcmp(pd_item_name(&S->game,729),"Dragonitita"));
  assert(!strcmp(pd_item_name(&S->game,764),"Crabominablita"));
  assert(*pd_item_description(&S->game,729));
 }
 assert(*pd_item_description(&S->game,13));
 assert(*pd_item_description(&S->game,728));
 assert(pd_type_multiplier_pct(&S->game,16,23,PD_TYPE_NONE)==0);
 assert(pd_type_multiplier_pct(&S->game,23,16,PD_TYPE_NONE)==200);

 ud_language_set(1);assert(!strcmp(ud_tr("PARTY"),"EQUIPO"));
 assert(!strcmp(ud_tr("Unbound Dual"),"Unbound Dual"));
 const char*marks="áéíóúÁÉÍÓÚñÑüÜ¿¡";
 for(const unsigned char*p=(const unsigned char*)marks;*p;){unsigned cp=*p++;if(cp>=0xc0)cp=(cp&31)<<6|(*p++&63);assert(pd_font_glyph(cp)->codepoint==cp);}
 uint32_t a[100*20]={0},b[100*20]={0};struct pd_canvas ca,cb;pd_canvas_init(&ca,100,20,a);pd_canvas_init(&cb,100,20,b);
 ud_language_set(0);pd_text_fit_raw(&ca,0,0,100,"BAG",0xffffff,PD_NO_SHADOW);ud_language_set(1);pd_text_fit_raw(&cb,0,0,100,"BAG",0xffffff,PD_NO_SHADOW);assert(!memcmp(a,b,sizeof(a)));
 assert(companion_update(1));assert(S->snap.partyCount==1&&S->snap.party[0].species==496&&S->snap.party[0].hp==34);
 companion_options(true,false);
 ud_language_set(0);S->dirty=true;companion_draw(english_frame);
 ud_language_set(1);S->dirty=true;
 companion_draw(frame);assert(memcmp(english_frame,frame,sizeof(frame)));screenshot(argv[3],"spanish-party");
 S->ui.summarySlot=0;S->dirty=true;companion_draw(frame);screenshot(argv[3],"spanish-summary");
 S->ui.summarySlot=-1;S->ui.tab=PD_TAB_BAG;S->dirty=true;companion_draw(frame);screenshot(argv[3],"spanish-bag");
 const enum pd_blob card_blobs[]={PD_FR_CARD_GFX,PD_FR_CARD_FRONT,PD_FR_CARD_BACK,PD_FR_CARD_BG,PD_FR_CARD_PAL1,PD_FR_FONT_NORMAL,PD_FR_FONT_NORMAL_WIDTHS,PD_CARD_STAR_PAL,PD_UB_PIC_MALE,PD_UB_PIC_MALE_PAL};
 for(unsigned i=0;i<sizeof(card_blobs)/sizeof(card_blobs[0]);i++)printf("card blob %d: %s\n",card_blobs[i],pd_romart_blob(&S->game,card_blobs[i])?"found":"MISSING");
 S->ui.tab=PD_TAB_MAP;S->dirty=true;companion_draw(frame);screenshot(argv[3],"spanish-map");
 S->ui.tab=PD_TAB_CARD;S->dirty=true;companion_draw(frame);screenshot(argv[3],"spanish-card");
 uint32_t card[PD_CARD_W*PD_CARD_H];struct pd_card_info info;assert(pd_card_info(&S->game,&S->snap,&info));assert(pd_card_render(&S->game,&info,false,true,false,card));
 ud_language_set(0);assert(!strcmp(ud_tr("PARTY"),"PARTY"));
 printf("%s: exact identity, active tables, moves/items, accented glyphs, baseline RAM decode, localized UI; %zu bytes\n",profile->label,companion_memory());
 // Battle fixture is also from English; verify common RAM decoding without writes.
 ram(argv[2],"unbound_battle","ewram",ewram,262144);ram(argv[2],"unbound_battle","iwram",iwram+32768,32768);
 unsigned char saved_ew[262144],saved_iw[32768];memcpy(saved_ew,ewram,sizeof(saved_ew));memcpy(saved_iw,iwram+32768,sizeof(saved_iw));
 S->ui.tab=PD_TAB_PARTY;S->dirty=true;assert(companion_update(10000));
 assert(S->snap.partyCount==1&&S->snap.party[0].species==496&&S->snap.party[0].hp==34);
 companion_draw(frame);screenshot(argv[3],"spanish-battle");
 assert(!memcmp(saved_ew,ewram,sizeof(saved_ew))&&!memcmp(saved_iw,iwram+32768,sizeof(saved_iw)));
 cp_adapter_close();assert(companion_memory()==0);
 assert(cp_adapter_open(argv[1]));assert(!strcmp(companion_cache_status(),"hit"));
 assert(pd_romart_blob(&S->game,PD_FR_FONT_NORMAL));cp_adapter_close();assert(companion_memory()==0);
 free(image);memset(memory_map_read,0,sizeof(memory_map_read));
 puts("ES real-ROM preparation and cleanup PASS (baseline RAM fixture; hardware save testing still required)");
}
