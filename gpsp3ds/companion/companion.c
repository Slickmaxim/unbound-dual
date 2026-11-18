#include "localization.h"
#include "companion.h"
#include "alloc.h"
#include "cache.h"
#include "progress.h"
#include "vendor/pd_card.h"
#include "vendor/pd_game.h"
#include "vendor/pd_snapshot.h"
#include "vendor/pd_settings.h"
#include "vendor/pd_ui_internal.h"
#include "vendor/pd_map.h"
#include "vendor/pd_tables.h"
#include "vendor/pd_unbound_profile.h"
#include "vendor/pd_unbound_ru.h"
#include "vendor/pd_font.h"
#include "vendor/pd_party_art.h"
#include "vendor/pd_romart.h"
#include <string.h>
#include <stdio.h>
#include "../source/ui.h"
void cp_ui_init(struct pd_ui*,struct pd_settings*);
void cp_ui_draw(struct pd_ui*,struct pd_canvas*,const struct pd_game*,const struct pd_snapshot*);
bool cp_ui_touch(struct pd_ui*,int,int,int);
void cp_map_reset(void);
bool cp_bag_draw_list(struct pd_ui*,struct pd_canvas*,int rect[4]);
void cp_art_reset(void);
void cp_party_reset(void);
void cp_card_reset(void);
int cp_bottom_smooth;
static bool cp_dark_mode;
static const struct companion_guest* item_guest;
void companion_item_bind(const struct companion_guest* guest){ item_guest=guest; }
static char cache_directory[700];
void companion_cache_directory(const char*d){snprintf(cache_directory,sizeof(cache_directory),"%s",d?d:"");}
static struct state {
    struct pd_game game;
    struct pd_snapshot snap;
    struct pd_ui ui;
    struct pd_settings settings;
    uint32_t pixels[320*240];
    cp_read_fn read;
    void *ctx;
    uint64_t next;
    unsigned interval;
    bool dirty, bag_list_only, item_profile, tm_profile;
    uint32_t target_personality[PD_PARTY_SIZE];
    int target_moves[PD_PARTY_SIZE][PD_NUM_MOVES];
    struct { int item, slot, move, frames; } item_request;
    const char *item_notice;
    unsigned item_notice_frames;
#ifdef COMPANION_NATIVE
    int pocket,scroll,detail,drag_pixels;
#endif
} *S;
static const char *error="Companion is not loaded";
static bool supported;
static const char*cache_status="not loaded";
const char*companion_cache_status(void){return cache_status;}
static bool item_crc(const struct pd_game* g, uint32_t address, uint32_t crc)
{
    uint8_t bytes[32];
    return address && pd_rom_read(g,address,bytes,sizeof(bytes)) &&
        pd_crc32(bytes,sizeof(bytes))==crc;
}

static bool item_profile_matches(const struct pd_game* g)
{
    const struct pd_config* cfg=g->cfg;
    const struct pd_party_fns* party=cfg->partyFns;
    uint8_t wait[24];
    return party && cfg->itemDescs && cfg->itemDescs->stride==44 &&
        pd_rom_read(g,0x08000890,wait,sizeof(wait)) &&
        pd_crc32(wait,sizeof(wait))==0xf11a078c &&
        item_crc(g,cfg->fieldCheckBag,cfg->fieldCrcs[2]) &&
        item_crc(g,cfg->fieldRemoveBag,cfg->fieldCrcs[3]) &&
        item_crc(g,cfg->fieldLocked,cfg->fieldCrcs[4]) &&
        item_crc(g,party->itemNoEffect,party->codeCrcs[2]) &&
        item_crc(g,party->itemEffect,party->codeCrcs[3]);
}

// FireRed's native party menu. Unbound keeps these entries, but the generic
// item-menu wrapper probes the medicine table before it chooses a pocket.
static const struct tm_entry { uint32_t address, crc; } tm_entries[]={
    {0x0811ea44,0x731f3e20}, // InitPartyMenu
    {0x0811fb28,0x1ccd94fd}, // choose Pokémon task
    {0x08125b40,0x9ca850aa}, // ItemUseCB_TMHM
    {0x080563f0,0xe67bc364}, // clean up field windows
    {0x080567dc,0xb892bbb7}, // return to field
    {0x0811ebd0,0x3099dc9b}, // initialize the party screen
    {0x08069940,0x0111a86e}, // lock field controls
};

static bool tm_profile_matches(const struct pd_game* g)
{
    for(unsigned i=0;i<sizeof(tm_entries)/sizeof(*tm_entries);++i)
        if(!item_crc(g,tm_entries[i].address,tm_entries[i].crc)) return false;
    return true;
}

static bool item_in(const uint16_t* ids,int count,int item)
{
    for(int i=0;i<count;++i) if(ids[i]==item) return true;
    return false;
}

static int item_kind(int id)
{
    if(!item_guest || !S || !S->item_profile || id<=0 || S->snap.inBattle)
        return 0;
    const struct pd_config* cfg=S->game.cfg;
    const struct pd_party_fns* p=cfg->partyFns;
    if(id>=p->itemCount) return 0;
    int kind=item_in(p->ppOne,p->ppOneCount,id) ? 2 :
        (item_in(p->medicine,p->medicineCount,id) ||
         item_in(p->ppAll,p->ppAllCount,id)) ? 1 : 0;
    bool tm=S->tm_profile && item_guest->call_stack && item_guest->write && id>=289 && id<=346;
    if(!kind && !tm) return 0;
    uint8_t record[32];
    if(!pd_rom_read(&S->game,cfg->itemDescs->base+44u*id,record,sizeof(record)) ||
        (record[14]|record[15]<<8)!=id) return 0;
    if(tm && record[26]==4) return 3;
    uint32_t callback=record[28]|(uint32_t)record[29]<<8|
        (uint32_t)record[30]<<16|(uint32_t)record[31]<<24;
    for(int i=0;i<p->useFuncCount;++i)
        if((callback&~1u)==p->useFuncs[i]) return kind;
    return 0;
}

bool cp_item_available(int item){ return item_kind(item)!=0; }

void companion_item_cancel(void)
{
    if(!S) return;
    S->item_request.frames=0;
    S->ui.itemPicking=false;
    S->ui.itemMoveSlot=-1;
    S->item_notice=NULL;
    S->item_notice_frames=0;
    companion_invalidate();
}

static void item_result(const char* notice)
{
    S->item_request.frames=0;
    S->item_notice=notice;
    S->item_notice_frames=120;
    S->next=0;
    companion_invalidate();
}

static bool item_call(uint32_t address,uint32_t a,uint32_t b,uint32_t c,uint32_t d,
                      uint32_t* out,bool query)
{
    uint32_t args[4]={a,b,c,d};
    return item_guest->call(address,args,out,query);
}

static void tm_open(int id)
{
    const struct pd_config* cfg=S->game.cfg;
    uint32_t result, callback=0;
    const char* message="Item use failed.";
    bool opened=false;
    const uint32_t menu[7]={
        0,0,3,0,4, // field, single layout, use item, reset cursor, teach prompt
        0x0811fb29,0x080567dd // choose Pokémon, return to field
    };
    uint8_t count=0;
    if(item_call(cfg->fieldLocked,0,0,0,0,&result,true)){
        if(result) message="Use items in the field.";
        else if(item_call(cfg->fieldCheckBag,id,1,0,0,&result,true)){
            if(result!=1) message="No items left.";
            else if(S->read(S->ctx,cfg->partyFns->partyCount,&count,1) && count>0 &&
                item_call(0x08069940,0,0,0,0,&result,false) &&
                item_call(0x080563f0,0,0,0,0,&result,false) &&
                item_guest->write(0x0203ad30,id,2) &&
                item_guest->write(0x03005e98,0x08125b41,4) &&
                item_guest->call_stack(0x0811ea44,menu,&result) &&
                S->read(S->ctx,cfg->gMain+4,&callback,4) && callback==0x0811ebd1){
                opened=true;
                message="Choose a Pokémon in the game.";
            }
        }
    }
    item_guest->end(opened);
    item_result(message);
}

bool companion_item_tick(void)
{
    if(!S || !item_guest) return false;
    if(S->item_notice_frames && !--S->item_notice_frames){
        S->item_notice=NULL;
        companion_invalidate();
    }
    if(!S->item_request.frames) return false;
    if(!--S->item_request.frames){
        item_result("Try again on the field.");
        return false;
    }
    return true;
}

void companion_item_at_wait(void)
{
    if(!S || !item_guest || !S->item_request.frames || !item_guest->idle())
        return;
    const struct pd_config* cfg=S->game.cfg;
    const struct pd_party_fns* p=cfg->partyFns;
    int id=S->item_request.item, slot=S->item_request.slot, move=S->item_request.move;
    uint8_t flags,count,raw[100];
    uint32_t callback;
    int kind=item_kind(id);
    if(!kind || !S->read(S->ctx,cfg->gMain+4,&callback,4) ||
       callback!=0x080565b5 || !S->read(S->ctx,cfg->gMain+0x439,&flags,1) || (flags&2)){
        item_result("Use items in the field.");return;
    }
    if(kind==3){
        if(item_guest->begin()) tm_open(id);
        return;
    }
    uint32_t mon=p->party+100u*slot;
    struct pd_mon live;
    if(!S->read(S->ctx,p->partyCount,&count,1) || slot>=count ||
       !S->read(S->ctx,mon,raw,sizeof(raw)) || !pd_decode_party_mon_for(cfg,raw,&live) ||
       live.egg || live.personality!=S->target_personality[slot] ||
       (kind==2 && live.moves[move]!=S->target_moves[slot][move])){
        item_result("The party changed.");return;
    }
    if(!item_guest->begin()) return;
    uint32_t result;
    const char* message="Item use failed.";
    bool commit=false;
    if(item_call(cfg->fieldLocked,0,0,0,0,&result,true)){
        if(result) message="Use items in the field.";
        else if(item_call(cfg->fieldCheckBag,id,1,0,0,&result,true)){
            if(result!=1) message="No items left.";
            else if(item_call(p->itemNoEffect,mon,id,slot,move,&result,true)){
                if(result) message="It would have no effect.";
                else if(item_call(p->itemEffect,mon,id,slot,move,&result,false) && result==0 &&
                        item_call(cfg->fieldRemoveBag,id,1,0,0,&result,false) && result==1){
                    commit=true;
                    message="Item used.";
                }
            }
        }
    }
    item_guest->end(commit);
    item_result(message);
}

static void item_touch_action(void)
{
    struct pd_ui* ui=&S->ui;
    int kind=ui->action==PD_ACTION_ITEM_PICK ? item_kind(ui->itemId) : 0;
    if(kind==3 && !S->item_request.frames){
        S->item_request.item=ui->itemId;
        S->item_request.frames=90;
    }else if(kind && !S->item_request.frames){
        ui->itemPicking=true;
        ui->itemMoveSlot=-1;
        memset(ui->itemSlots,0,sizeof(ui->itemSlots));
        for(int i=0;i<S->snap.partyCount;++i){
            const struct pd_mon* mon=&S->snap.party[i];
            ui->itemSlots[i].ok=!mon->egg;
            S->target_personality[i]=mon->personality;
            memcpy(S->target_moves[i],mon->moves,sizeof(mon->moves));
            if(kind==2)
                for(int m=0;m<PD_NUM_MOVES;++m) ui->itemSlots[i].moves[m]=mon->moves[m]>0;
        }
    }else if(ui->action==PD_ACTION_ITEM_APPLY && !S->item_request.frames &&
             ui->itemSlot>=0 && ui->itemSlot<PD_PARTY_SIZE && ui->itemMove>=0 && ui->itemMove<PD_NUM_MOVES){
        S->item_request.item=ui->itemId;
        S->item_request.slot=ui->itemSlot;
        S->item_request.move=ui->itemMove;
        S->item_request.frames=90;
    }
    ui->action=PD_ACTION_NONE;
}

void companion_close(void){
    supported=false;
    pd_font_set(NULL, 0);
    cp_card_reset();cp_map_reset();cp_party_art_reset();cp_art_reset();cp_party_reset();cp_free_all();S=NULL;
    error="Companion is not loaded";
}
bool companion_load(const uint8_t header[192],size_t size,cp_read_fn read,
                    const uint8_t*(*span)(void*,uint32_t,size_t),void*ctx){
    companion_close();
    cache_status="unavailable";
    struct pd_game game;
    pd_game_detect(&game,header,size);
    game.rom=header; // presence token; every ROM access uses the adapter
    game.romSize=size;game.romRead=read;game.romAt=span;game.romCtx=ctx;
    pd_game_identify(&game);
    if(strcmp(game.sha1,"b4776b82a4c7915d0fadeaa27e013523f99dfd94") &&
       !pd_unbound_profile(game.sha1)){
        error="Requires a supported Unbound EN/ES/RU ROM";return false;
    }
    if(!game.cfg){error="Unbound layout unavailable";return false;}
    S=cp_calloc(1,sizeof(*S));if(!S){error="Not enough memory for Companion";return false;}
    S->game=game;
    if (game.cfg->cyrillic && !pd_ru_load_text(&S->game)) {
        companion_close();
        error="Could not load Russian ROM text";
        return false;
    }
    S->item_profile=item_profile_matches(&S->game);
    S->tm_profile=S->item_profile && tm_profile_matches(&S->game);
    S->read=read;S->ctx=ctx;S->interval=200;
#ifdef COMPANION_NATIVE
    S->detail=-1;
#endif
    pd_settings_defaults(&S->settings);
    S->settings.darkMode=cp_dark_mode;
    pd_look_apply(&S->game,&S->settings);
    cp_ui_init(&S->ui,&S->settings);
    // Asset discovery happens with emulation paused, never in the game loop.
    cp_progress("Reading cache",0,0);
    // Older ES caches can be valid yet lack a translated font/status sheet,
    // which used to disable all slot artwork. Rebuild those once on upgrade.
    if(!cp_cache_load(cache_directory,&S->game) || !pd_romart_party_ready(&S->game)){
        cp_art_reset();cp_map_reset();cp_allocation_clear_failure();
        cp_progress("Finding graphics",0,0);
        pd_romart_scan(&S->game);
        cp_progress("Preparing map and card",0,0);
        pd_map_art(&S->game);
        if(!cp_allocation_failed()){
            cp_progress("Saving cache",0,0);
            cache_status=cp_cache_save(cache_directory,&S->game)?"rebuilt":"not saved";
        }
    }else cache_status="hit";
    if(cp_allocation_failed()){companion_close();error="Not enough memory for Companion";return false;}
    if (S->game.cfg->cyrillic) pd_ru_use_font(&S->game);
    supported=true;error=NULL;S->dirty=true;S->bag_list_only=false;return true;
}
void companion_options(bool dark,bool smooth){
 cp_dark_mode=dark;cp_bottom_smooth=smooth;
 if(S){S->settings.darkMode=dark;pd_look_apply(&S->game,&S->settings);S->dirty=true;S->bag_list_only=false;}
}
bool companion_supported(void){return supported;}
const char*companion_error(void){return error;}
void companion_invalidate(void){if(S){S->dirty=true;S->bag_list_only=false;}}
bool companion_bag_open(void){return supported&&S&&S->ui.tab==PD_TAB_BAG;}
unsigned companion_interval_ms(void){return S?S->interval:1000;}
size_t companion_memory(void){return cp_memory_used();}
void companion_feedback(unsigned us){if(S&&us>3000&&S->interval<1000)S->interval=S->interval<500?500:1000;}
// Compare only data used by the current tab. VBlank counters, play time,
// battle bookkeeping and guide flags must not invalidate an unchanged page.
static bool visible_changed(const struct pd_snapshot *a, const struct pd_snapshot *b, enum pd_tab tab) {
    if(a->valid!=b->valid)return true;
    if(tab==PD_TAB_PARTY)return a->partyCount!=b->partyCount || memcmp(a->party,b->party,sizeof(a->party));
    if(tab==PD_TAB_BAG)return a->bagOk!=b->bagOk || memcmp(a->bag,b->bag,sizeof(a->bag));
    if(tab==PD_TAB_CARD){
        struct pd_card_info x,y;bool ax=pd_card_info(&S->game,a,&x),by=pd_card_info(&S->game,b,&y);
        return ax!=by||(ax&&memcmp(&x,&y,sizeof(x)));
    }
    return a->mapsec!=b->mapsec || a->posOk!=b->posOk || a->x!=b->x || a->y!=b->y ||
        a->mapGroup!=b->mapGroup || a->mapNum!=b->mapNum || a->mapW!=b->mapW || a->mapH!=b->mapH ||
        a->mapType!=b->mapType || a->gender!=b->gender;
}
bool companion_update(uint64_t now){
    if(!supported)return false;
    if(now>=S->next){
        struct pd_snapshot snap;
        pd_snapshot_read(&snap,&S->game,S->read,S->ctx);
        if(visible_changed(&snap,&S->snap,S->ui.tab)){S->dirty=true;S->bag_list_only=false;}
        S->snap=snap;
        S->next=now+(uint64_t)S->interval*1000;
    }
    return S->dirty;
}
#ifdef COMPANION_NATIVE
static void native_text(int x,int y,const char*t){ui_text(&ui_font_body,x,y,t,UI_RGB(230,233,240),300-x);}
static void native_wrap(const char *text, int y) {
    if (!text) return;
    while (*text && y < 190) {
        char line[256]; size_t n=0; const char *start=text, *last_space=NULL;
        while (*text && n<sizeof(line)-5 && *text!='\n') {
            const char *next=text;ui_utf8_next(&next);
            size_t bytes=(size_t)(next-text);
            memcpy(line+n,text,bytes);line[n+bytes]=0;
            if(ui_text_w(&ui_font_body,line)>290)break;
            if(*text==' ')last_space=text;
            n+=bytes;text=next;
        }
        if(last_space && *text && *text!='\n'){text=last_space+1;n=(size_t)(last_space-start);}
        if(!n){if(*text)++text;continue;}
        line[n]=0;native_text(12,y,line);y+=20;
        if(*text=='\n')++text;
        while(*text==' ')++text;
    }
}
static void native_draw(void){
    struct pd_ui*u=&S->ui;const struct pd_snapshot*s=&S->snap;char b[128],name[80];
    ui_clear(UI_RGB(14,17,23));u->hitCount=0;u->drawn=s;u->drawnGame=&S->game;
    if(!s->valid){native_text(12,70,"Waiting for game data...");}
    else if(u->tab==PD_TAB_PARTY){
        if(u->summarySlot>=0&&u->summarySlot<s->partyCount){
            const struct pd_mon*m=&s->party[u->summarySlot];
            native_text(12,5,m->nickname);
            snprintf(b,sizeof(b),ud_tr("Lv %d    HP %d/%d"),m->level,m->hp,m->maxHp);native_text(12,29,b);
            for(int i=0;i<4;++i){snprintf(b,sizeof(b),"%s   PP %d",pd_move_name(&S->game,m->moves[i],name,sizeof(name)),m->pp[i]);native_text(12,54+i*22,b);}
            snprintf(b,sizeof(b),ud_tr("Atk %d  Def %d  Spd %d"),m->stats.values[1],m->stats.values[2],m->stats.values[3]);native_text(12,148,b);
            snprintf(b,sizeof(b),ud_tr("SpA %d  SpD %d"),m->stats.values[4],m->stats.values[5]);native_text(12,170,b);
            ui_text(&ui_font_small,245,5,"BACK",UI_RGB(130,180,230),70);ui_add_hit(u,240,0,80,25,HIT_BACK);
        }else for(int i=0;i<s->partyCount&&i<6;++i){
            const struct pd_mon*m=&s->party[i];int y=5+i*33;
            ui_rrect(6,y,308,30,5,UI_RGB(28,34,47));
            ui_text(&ui_font_bold,13,y+1,m->nickname,UI_RGB(230,233,240),170);
            snprintf(b,sizeof(b),ud_tr("Lv%d  %d/%d"),m->level,m->hp,m->maxHp);
            ui_text_right(&ui_font_small,305,y+4,b,UI_RGB(150,210,180));
            int w=m->maxHp>0?280*m->hp/m->maxHp:0;
            ui_fill(14,y+24,280,3,UI_RGB(60,65,75));if(w>0)ui_fill(14,y+24,w,3,UI_RGB(90,205,145));
            if(!m->egg&&(m->hp<=0||m->status))ui_text_right(&ui_font_small,305,y+13,m->hp<=0?"FNT":pd_status_label(m->status),UI_RGB(240,165,110));
            ui_add_hit(u,6,y,308,30,HIT_SLOT+i);
        }
    }else if(u->tab==PD_TAB_BAG){
        if(S->pocket<0||S->pocket>=PD_POCKET_COUNT)S->pocket=0;
        const struct pd_pocket_items*p=&s->bag[S->pocket];
        snprintf(b,sizeof(b),"<  %s  >",pd_pocket_name(S->pocket));native_text(12,4,b);
        ui_add_hit(u,0,0,150,27,9100);ui_add_hit(u,150,0,170,27,9101);
        int max=p->count>6?p->count-6:0;if(S->scroll>max)S->scroll=max;
        if (S->detail>=0 && S->detail<p->count) {
            native_text(12,36,pd_item_name(&S->game,p->items[S->detail].id));
            snprintf(b,sizeof(b),ud_tr("Quantity: %u"),p->items[S->detail].quantity);native_text(12,60,b);
            native_wrap(pd_item_description(&S->game,p->items[S->detail].id),90);
            ui_text(&ui_font_small,250,191,"BACK",UI_RGB(160,195,235),65);
            ui_add_hit(u,240,186,75,22,9104);
        } else {
            for(int j=0;j<6&&j+S->scroll<p->count;++j){int i=j+S->scroll,y=31+j*21;
                ui_rrect(7,y,306,20,3,UI_RGB(25,31,42));
                ui_text(&ui_font_body,12,y+1,pd_item_name(&S->game,p->items[i].id),UI_RGB(230,233,240),245);
                snprintf(b,sizeof(b),"%u",p->items[i].quantity);ui_text_right(&ui_font_body,305,y+1,b,UI_RGB(140,210,180));
                ui_add_scroll_hit(u,7,y,306,20,9200+i);
            }
            ui_text(&ui_font_small,240,189,"<   >",UI_RGB(160,195,235),75);
            ui_add_hit(u,235,185,38,23,9102);ui_add_hit(u,273,185,42,23,9103);
        }
    }else{
        struct pd_canvas c;pd_canvas_init(&c,320,208,S->pixels);pd_fill(&c,0,0,320,208,0x111722);
        ui_game=&S->game;struct pd_host_info host={0};
        if(u->overlay==PD_OVERLAY_PLACES)ui_places_overlay(u,&c,&S->game);else ui_map_tab(u,&c,&S->game,s,&host);
        for(int i=0;i<320*208;++i){uint32_t v=S->pixels[i];ui_canvas[i]=UI_RGB(v>>16,v>>8,v);}
    }
    static const char*names[]={"PARTY","BAG","MAP","MENU"};
    for(int i=0;i<4;++i){int x=4+i*79;ui_rrect(x,212,75,25,5,UI_RGB(36,49,69));ui_text_center(&ui_font_small,x+37,218,names[i],UI_RGB(220,230,244));ui_add_hit(u,x,212,75,25,9000+i);}
}
#endif
void companion_draw(uint16_t*out){
    if(!supported){
        ui_clear(UI_RGB(18,25,37));
        ui_text_center(&ui_font_bold,160,40,"Companion",UI_RGB(230,235,245));
        ui_text(&ui_font_small,10,80,error?error:"Unavailable",UI_RGB(200,210,225),300);
        ui_rrect(240,212,75,24,5,UI_RGB(36,49,69));
        ui_text_center(&ui_font_small,278,218,"MENU",UI_RGB(230,235,245));
        if(out!=ui_canvas)memcpy(out,ui_canvas,sizeof(ui_canvas));
        return;
    }

#ifdef COMPANION_NATIVE
    native_draw();if(out!=ui_canvas)memcpy(out,ui_canvas,sizeof(ui_canvas));
#else
    struct pd_canvas c;pd_canvas_init(&c,320,240,S->pixels);cp_ui_draw(&S->ui,&c,&S->game,&S->snap);
    if(S->item_notice&&S->ui.tab==PD_TAB_BAG){
        pd_fill(&c,0,188,320,20,0x202631);
        pd_text_fit(&c,8,189,304,S->item_notice,0xffffff,PD_NO_SHADOW);
    }
    for(int i=0;i<320*240;++i){uint32_t v=S->pixels[i];out[i]=((v>>19)<<11)|(((v>>10)&63)<<5)|((v>>3)&31);}
#endif
    S->dirty=S->bag_list_only=false;
}
bool companion_draw_bag_scroll(uint16_t* out,int rect[4]){
#ifndef COMPANION_NATIVE
    if(!companion_bag_open() || !S->bag_list_only || S->ui.itemPicking || S->item_notice) return false;
    struct pd_canvas c;
    pd_canvas_init(&c,320,208,S->pixels);
    if(!cp_bag_draw_list(&S->ui,&c,rect)) return false;
    for(int y=rect[1];y<rect[1]+rect[3];++y)
        for(int x=rect[0];x<rect[0]+rect[2];++x){
            int i=y*320+x;
            uint32_t v=S->pixels[i];
            out[i]=((v>>19)<<11)|(((v>>10)&63)<<5)|((v>>3)&31);
        }
    S->dirty=S->bag_list_only=false;
    return true;
#else
    (void)out;(void)rect;
    return false;
#endif
}
bool companion_touch(int phase,int x,int y){
    if(!supported)return phase==2&&y>=212&&x>=240;
#ifdef COMPANION_NATIVE
    if(phase==2&&S->ui.touching&&!S->ui.dragging){
        int id=S->ui.pressedId;
        if(id==9104)S->detail=-1;
        if(id>=9100&&id<=9103){
            if(id<=9101){S->pocket=(S->pocket+PD_POCKET_COUNT+(id==9100?-1:1))%PD_POCKET_COUNT;S->scroll=0;S->detail=-1;}
            else {S->scroll+=id==9102?-6:6;if(S->scroll<0)S->scroll=0;}
        }
        if(id>=9200&&id<9200+PD_POCKET_SLOTS)S->detail=id-9200;
    }
#endif
    int previous_id=S->ui.pressedId, previous_y=S->ui.touchLastY;
    int previous_scroll=S->ui.bagScroll;
    bool menu=cp_ui_touch(&S->ui,phase,x,y);
    if(phase==2) item_touch_action();
#ifdef COMPANION_NATIVE
    if(phase==1 && S->ui.dragging && S->ui.tab==PD_TAB_BAG) {
        S->drag_pixels+=previous_y-y;
        while(S->drag_pixels>=21){++S->scroll;S->drag_pixels-=21;}
        while(S->drag_pixels<=-21){--S->scroll;S->drag_pixels+=21;}
        if(S->scroll<0)S->scroll=0;
    }
    if(phase==2)S->drag_pixels=0;
#endif
    if(phase==1 && S->ui.tab==PD_TAB_BAG){
        if(previous_scroll!=S->ui.bagScroll || previous_id!=S->ui.pressedId){
            bool list_press=previous_id<0 || previous_id==HIT_BAG_LIST ||
                previous_id==HIT_SCROLL_BAR || (previous_id>=HIT_BAG_ROW && previous_id<HIT_BAG_ROW_END);
            S->bag_list_only=list_press && (!S->dirty||S->bag_list_only);
            S->dirty=true;
        }
    }else if(phase!=1 || previous_y!=y || previous_id!=S->ui.pressedId){
        companion_invalidate();
    }
    return menu;
}
