// Read-only host for the three original PokeDaisy tab components.
#include "vendor/pd_ui_internal.h"
#include <string.h>
const struct pd_game* ui_game;
void cp_ui_init(struct pd_ui *ui,struct pd_settings *settings){
    memset(ui,0,sizeof(*ui));ui->settings=settings;ui->tab=PD_TAB_PARTY;
    ui->summarySlot=ui->bagSelected=ui->pressedId=ui->focusId=-1;
    ui->mapPage=ui->mapSel=ui->mapDungeon=-1;
    ui->handheldParty=true;
}
bool ui_summary_learn(struct pd_ui* ui){(void)ui;return false;}
bool pd_ui_back(struct pd_ui*ui){
    if(ui->summarySlot>=0){ui->summarySlot=-1;return true;}
    if(ui->overlay!=PD_OVERLAY_NONE){ui->overlay=PD_OVERLAY_NONE;return true;}return false;
}
void cp_ui_draw(struct pd_ui *ui,struct pd_canvas*c,const struct pd_game*g,const struct pd_snapshot*s){
    ui_game=g;ui->drawn=s;ui->drawnGame=g;ui->hitCount=0;
    pd_backdrop(c);
    struct pd_canvas content=*c;content.h=208;content.cy1=208;
    struct pd_host_info host={0};
    if(ui->overlay==PD_OVERLAY_PLACES)ui_places_overlay(ui,c,g);
    else if(ui->tab==PD_TAB_PARTY)ui_party_tab(ui,&content,g,s,&host);
    else if(ui->tab==PD_TAB_BAG)ui_bag_tab(ui,&content,g,s,&host);
    else if(ui->tab==PD_TAB_CARD)ui_card_tab(ui,&content,g,s);
    else ui_map_tab(ui,&content,g,s,&host);
    static const char*names[]={"PARTY","BAG","MAP","CARD","MENU"};
    for(int i=0;i<5;++i)ui_button(ui,c,2+i*64,212,60,24,names[i],9000+i);
}
static void activate(struct pd_ui*ui,int id){
    if(id>=9000&&id<9004){static const enum pd_tab t[]={PD_TAB_PARTY,PD_TAB_BAG,PD_TAB_MAP,PD_TAB_CARD};ui->tab=t[id-9000];ui->overlay=PD_OVERLAY_NONE;ui->summarySlot=-1;return;}
    if(id==HIT_BACK){pd_ui_back(ui);return;}
    if(id>=HIT_SLOT&&id<HIT_SLOT+6){ui->summarySlot=id-HIT_SLOT;ui->summaryStats=ui->summaryInfo=false;return;}
    if(id>=HIT_SUMMARY_MON&&id<HIT_SUMMARY_MON+6){ui->summarySlot=id-HIT_SUMMARY_MON;return;}
    if(id==HIT_SUMMARY_STATS){ui->summaryStats=!ui->summaryStats;ui->summaryInfo=false;return;}
    if(id==HIT_SUMMARY_INFO){ui->summaryInfo=!ui->summaryInfo;ui->summaryStats=false;return;}
    if(id>=HIT_BAG_TAB&&id<HIT_BAG_LIST){ui->bagPocket=id-HIT_BAG_TAB;ui->bagSelected=-1;ui->bagScroll=0;return;}
    if(id>=HIT_BAG_ROW&&id<HIT_BAG_ROW_END){ui->bagSelected=id-HIT_BAG_ROW;return;}
    if(id==HIT_BAG_SORT){pd_ui_bag_cycle_sort(ui);return;}
    if(ui->tab==PD_TAB_MAP)ui_map_act(ui,id);
    if(ui->tab==PD_TAB_CARD)ui_card_act(ui,id);
    ui->action=PD_ACTION_NONE; // No host mutation API exists.
}
bool cp_ui_touch(struct pd_ui*ui,int phase,int x,int y){
    if(phase==2){if(!ui->touching)return false;ui->touching=false;
        int id=ui->pressedId;ui->pressedId=-1;
        if(ui->barDragging){ui->barDragging=false;ui->dragging=false;return false;}
        if(!ui->dragging&&id==9004)return true;
        if(!ui->dragging)activate(ui,id);ui->dragging=false;return false;}
    int id=-1;bool scroll=false;
    for(int i=ui->hitCount-1;i>=0;--i){const struct pd_hit*h=&ui->hits[i];if(x>=h->x&&x<h->x+h->w&&y>=h->y&&y<h->y+h->h){id=h->id;scroll=h->scroll;break;}}
    if(!ui->touching){ui->touching=true;ui->dragging=false;ui->touchScroll=scroll;ui->touchX0=x;ui->touchY0=ui->touchLastY=y;ui->pressedId=id;ui->barDragging=id==HIT_SCROLL_BAR;}
    else{
        int *scroll_target=ui->overlay==PD_OVERLAY_PLACES?&ui->placesScroll:ui->tab==PD_TAB_BAG?&ui->bagScroll:NULL;
        if(ui->barDragging){int span=ui->barH-ui->barThumb;
            if(scroll_target&&span>0)*scroll_target=ui_clamp_scroll((y-ui->barY-ui->barThumb/2)*ui->barMax/span,ui->barMax,0);
            ui->dragging=true;return false;}
        if(ui->touchScroll&&(y-ui->touchY0>5||ui->touchY0-y>5))ui->dragging=true;
        if(ui->dragging&&scroll_target){*scroll_target-=y-ui->touchLastY;if(*scroll_target<0)*scroll_target=0;}
        if(id!=ui->pressedId)ui->pressedId=-1;
        ui->touchLastY=y;
    }return false;
}
