#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
extern unsigned char ewram[],iwram[],*memory_map_read[];
extern uint32_t reg[];
extern bool gpsp_guest_begin(void);
extern bool gpsp_guest_call(uint32_t,const uint32_t[4],uint32_t*,bool);
extern void gpsp_guest_end(bool);
extern bool gpsp_guest_call_stack(uint32_t,const uint32_t[7],uint32_t*);
extern bool gpsp_guest_write(uint32_t,uint32_t,unsigned);
static void load(const char*path,void*out,size_t n){
 FILE*f=fopen(path,"rb");assert(f&&fread(out,1,n,f)==n);fclose(f);
}
int main(int argc,char**argv){
 assert(argc==4);
 unsigned char*rom=malloc(32*1024*1024);assert(rom);
 load(argv[1],rom,32*1024*1024);load(argv[2],ewram,256*1024);load(argv[3],iwram+0x8000,32*1024);
 for(int i=0;i<1024;i++)memory_map_read[4096+i]=rom+i*32768;
 for(int i=0;i<8;i++)memory_map_read[1024+i]=ewram+i*32768;
 memory_map_read[1536]=iwram+0x8000;
 reg[13]=0x03007e00;reg[15]=0x08000100;reg[16]=0x3f;reg[17]=0x10;
 const uint32_t mon=0x02024284;
 uint32_t r;
 const uint32_t setpp[]={mon,17,0x0203fff0,0}, getpp[]={mon,17,0,0};
 const uint32_t potion[]={13,1,0,0}, heal[]={mon,13,0,0};
 assert(gpsp_guest_begin());
 assert(gpsp_guest_call(134489108,heal,&r,true)&&r==1);
 gpsp_guest_end(false);
 assert(ewram[0x3bb22]==2);
 ewram[0x24284+0x56]=10;
 assert(gpsp_guest_begin());
 assert(gpsp_guest_call(134848320,potion,&r,true)&&r==1);
 assert(gpsp_guest_call(134489108,heal,&r,true)&&r==0);
 assert(ewram[0x24284+0x56]==10);
 assert(gpsp_guest_call(134484928,heal,&r,false)&&r==0);
 assert(gpsp_guest_call(134848984,potion,&r,false)&&r==1);
 gpsp_guest_end(true);
 assert(ewram[0x24284+0x56]==30 && ewram[0x3bb22]==1);
 puts("Potion: HP 10->30; quantity 2->1. Full HP query preserves quantity.");
 ewram[0x24284+0x56]=10;
 ewram[0x3c61c]=139;ewram[0x3c61e]=2;
 const uint32_t oran[]={139,1,0,0}, berry[]={mon,139,0,0};
 assert(gpsp_guest_begin());
 assert(gpsp_guest_call(134848320,oran,&r,true)&&r==1);
 assert(gpsp_guest_call(134489108,berry,&r,true)&&r==0);
 assert(gpsp_guest_call(134484928,berry,&r,false)&&r==0);
 assert(gpsp_guest_call(134848984,oran,&r,false)&&r==1);
 gpsp_guest_end(true);
 assert(ewram[0x24284+0x56]==20 && ewram[0x3c61e]==1);
 puts("Oran Berry: HP 10->20; quantity 2->1.");
 ewram[0x3fff0]=1;ewram[0x3bb2c]=34;ewram[0x3bb2e]=2;
 const uint32_t ether[]={34,1,0,0}, restore_pp[]={mon,34,0,0};
 assert(gpsp_guest_begin());
 assert(gpsp_guest_call(134480764,setpp,&r,false));
 assert(gpsp_guest_call(134478824,getpp,&r,true)&&r==1);
 assert(gpsp_guest_call(134848320,ether,&r,true)&&r==1);
 assert(gpsp_guest_call(134489108,restore_pp,&r,true)&&r==0);
 assert(gpsp_guest_call(134484928,restore_pp,&r,false)&&r==0);
 assert(gpsp_guest_call(134848984,ether,&r,false)&&r==1);
 assert(gpsp_guest_call(134478824,getpp,&r,true)&&r==11);
 gpsp_guest_end(true);
 puts("Ether: PP 1->11; quantity 2->1.");
 const uint32_t zero[4]={0}, tmargs[4]={289,1,0,0};
 const uint32_t menu[7]={0,0,3,0,4,0x0811fb29,0x080567dd};
 ewram[0x3c41c]=289&255;ewram[0x3c41d]=289>>8;ewram[0x3c41e]=1;
 uint32_t previous_callback,callback,exit_callback;
 memcpy(&previous_callback,iwram+0xb0f4,4);
 assert(gpsp_guest_begin());
 assert(gpsp_guest_call(134848320,tmargs,&r,true)&&r==1);
 assert(gpsp_guest_call(0x08069940,zero,&r,false));
 assert(gpsp_guest_call(0x080563f0,zero,&r,false));
 assert(gpsp_guest_write(0x0203ad30,289,2));
 assert(gpsp_guest_write(0x03005e98,0x08125b41,4));
 assert(gpsp_guest_call_stack(0x0811ea44,menu,&r));
 memcpy(&callback,iwram+0xb0f4,4);
 memcpy(&exit_callback,ewram+0x3b0a0,4);
 assert(callback==0x0811ebd1 && exit_callback==0x080567dd);
 assert(gpsp_guest_call(134848320,tmargs,&r,true)&&r==1);
 gpsp_guest_end(false);
 memcpy(&callback,iwram+0xb0f4,4);
 assert(callback==previous_callback);
 puts("TM01: native party-menu callback, field return and no consumption on opening.");
 free(rom);
}
