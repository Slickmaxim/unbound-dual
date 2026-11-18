/* Synthetic ROM snippets exercise the bounded guest-call interpreter. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint32_t reg[64];
extern unsigned char ewram[];
extern unsigned char *memory_map_read[];
bool gpsp_guest_begin(void);
bool gpsp_guest_call(uint32_t address, const uint32_t args[4], uint32_t *out, bool query);
void gpsp_guest_end(bool commit);

int main(void)
{
    static uint16_t rom[16384];
    // str r1,[r0]; movs r0,#7; bx lr
    rom[0]=0x6001; rom[1]=0x2007; rom[2]=0x4770;
    // str r1,[r0]; b .
    rom[8]=0x6001; rom[9]=0xe7fe;
    memory_map_read[0x08000000>>15]=(unsigned char*)rom;
    reg[13]=0x03007d00; reg[15]=0x08000100;
    reg[16]=0x3f; reg[17]=0x10;
    uint32_t saved[64]; memcpy(saved,reg,sizeof(saved));
    uint32_t args[4]={0x02000000,0x12345678,0,0}, result=0;
    assert(gpsp_guest_begin());
    assert(gpsp_guest_call(0x08000000,args,&result,true) && result==7);
    assert(ewram[0]==0);
    assert(gpsp_guest_call(0x08000000,args,&result,false));
    assert(ewram[0]==0x78);
    assert(!gpsp_guest_call(0x08000010,args,&result,false));
    gpsp_guest_end(true);
    assert(ewram[0]==0 && !memcmp(saved,reg,sizeof(saved)));

    assert(gpsp_guest_begin());
    assert(gpsp_guest_call(0x08000000,args,&result,false));
    gpsp_guest_end(true);
    assert(ewram[0]==0x78 && !memcmp(saved,reg,sizeof(saved)));

    args[0]=0x04000208;
    assert(gpsp_guest_begin());
    assert(!gpsp_guest_call(0x08000000,args,&result,false));
    gpsp_guest_end(false);
    assert(!memcmp(saved,reg,sizeof(saved)));
    puts("Guest calls: result, context restoration, query rollback, timeout rollback, commit, and I/O rejection passed");
}
