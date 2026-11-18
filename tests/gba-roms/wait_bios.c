#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "libretro.h"
extern uint32_t reg[64],execute_cycles,skip_next_frame;
extern unsigned char bios_rom[],open_gba_bios_rom[],*memory_map_read[];
extern void init_memory(void),init_main(void),init_cpu(void),execute_arm(uint32_t);
extern void gpsp_guest_set_wait_hook(void (*hook)(void));
static unsigned calls;
static void on_wait(void) { ++calls; }
static void video(const void*p,unsigned w,unsigned h,size_t n) {(void)p;(void)w;(void)h;(void)n;}
int main(void){
 static unsigned char rom[32768];
 memcpy(bios_rom,open_gba_bios_rom,16384);
 init_memory();init_cpu();init_main();retro_set_video_refresh(video);
 memory_map_read[4096]=rom;
 rom[0x89a]=2;rom[0x89b]=0xdf;
 reg[15]=0x0800089a;reg[16]=0x3f;reg[17]=0x10;
 skip_next_frame=1;
 gpsp_guest_set_wait_hook(on_wait);
 execute_arm(execute_cycles);
 printf("BIOS wait calls=%u\n",calls);
 assert(calls==1);
}
