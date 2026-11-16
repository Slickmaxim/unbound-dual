#include "../adapter.c"
#include "../cache.h"
#include <unistd.h>
void cp_art_reset(void);
void cp_map_reset(void);
#include "../vendor/pd_game.h"
#include "../vendor/pd_map.h"
#include "../vendor/pd_games.h"
#include "../../source/ui.h"
#define malloc cp_malloc
#define free cp_free
#include "../vendor/pd_romart.c"
#undef malloc
#undef free
#include <assert.h>
#include <stdio.h>
unsigned char ewram[512*1024],iwram[64*1024];
unsigned char *memory_map_read[8192];
static void word(uint8_t*p,unsigned at,uint32_t v){for(int i=0;i<4;++i)p[at+i]=(uint8_t)(v>>(i*8));}
static size_t literal_lz(uint8_t*p,const uint8_t*data,size_t n){p[0]=0x10;p[1]=(uint8_t)n;p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)(n>>16);size_t at=4;for(size_t i=0;i<n;i+=8){p[at++]=0;size_t k=n-i<8?n-i:8;memcpy(p+at,data+i,k);at+=k;}return at;}
int main(int argc,char**argv){assert(argc==2);
 // Two deliberately separated resident ROM blocks: spans must cross safely.
 uint8_t *a=malloc(1024*1024+4096),*b=malloc(1024*1024+4096);assert(a&&b);
 for(unsigned i=0;i<1024*1024;++i){a[i]=(uint8_t)(i^i>>12);b[i]=(uint8_t)(i^i>>9);}
 for(unsigned i=0;i<32;++i){memory_map_read[4096+i]=a+i*32768;memory_map_read[4128+i]=b+i*32768;}
 R.size=2*1024*1024;
 const uint8_t *cross=span(NULL,0x080FFFF0,128*1024);assert(cross);
 assert(!memcmp(cross,a+0xFFFF0,16));assert(!memcmp(cross+16,b,48));
 uint8_t *copy=cp_malloc(128*1024);assert(copy);assert(read_bus(NULL,0x080FFFF0,copy,128*1024));assert(!memcmp(cross,copy,128*1024));cp_free(copy);
 uint8_t outside;assert(!read_bus(NULL,0x08200000,&outside,1));
 assert(!cp_malloc(CP_MEMORY_LIMIT));cp_adapter_close();assert(!companion_memory());
 // Original synthetic FireRed-shaped region map, with no commercial assets.
 uint8_t *rom=calloc(1,2*1024*1024);assert(rom);
 word(rom,0xC0330,0x08180000);word(rom,0xC02EC,0x08181000);
 const unsigned maps[4]={0xC035C,0xC0370,0xC0388,0xC03A4};
 const unsigned grids[4]={0xC4194,0xC419C,0xC41A4,0xC41CC};
 for(int i=0;i<4;++i){word(rom,maps[i],0x08182000);word(rom,grids[i],0x08186000);}
 word(rom,0xC0C94,0x08183000);word(rom,0xC3D3C,0x08184000);word(rom,0xC3D38,0x08185000);word(rom,0xC00BC,0x08187000);
 rom[0xC0064]=0xC4;rom[0xC0065]=0x28;rom[0xC0067]=0xD9;
 uint8_t tiles[32]={0},map[30*20*2]={0};literal_lz(rom+0x180000,tiles,sizeof(tiles));literal_lz(rom+0x182000,map,sizeof(map));
 // One named section with a nonzero rectangle; other names are empty.
 for(int i=0;i<109;++i)word(rom,0x183000+i*4,0x08188010);
 word(rom,0x183000,0x08188000);rom[0x188000]=0xCE;rom[0x188001]=0xBF;rom[0x188002]=0xCD;rom[0x188003]=0xCE;rom[0x188004]=0xFF;rom[0x188010]=0xFF;
 rom[0x184000]=1;rom[0x184002]=2;rom[0x185000]=2;rom[0x185002]=1;
 memset(rom+0x186000,0xC5,2*22*15);rom[0x186000]=0x58;memset(rom+0x187000,0xC5,90);
 FILE*f=fopen(argv[1],"wb");assert(f);assert(fwrite(rom,1,2*1024*1024,f)==2*1024*1024);assert(!fclose(f));
 R.file=fopen(argv[1],"rb");assert(R.file);R.size=2*1024*1024;R.preparing=true;
 uint8_t header[192]={0};memcpy(header+0xAC,"BPRE",4);header[0xB2]=0x96;
 struct pd_game g;pd_game_detect(&g,header,R.size);g.kind=PD_GAME_FIRERED;g.rom=header;g.romSize=R.size;g.romAt=span;g.romRead=read_bus;
 g.entry=pd_game_entry_for("b4776b82a4c7915d0fadeaa27e013523f99dfd94");assert(g.entry&&g.entry->fieldCfg);g.cfg=g.entry->fieldCfg;
 const struct pd_map_art*art=pd_map_art(&g);assert(art&&art->ok&&art->pages==1);
 assert(art->w==240&&art->h==160);assert(companion_memory()<CP_MEMORY_LIMIT);
 // Round-trip real prepared synthetic map through disk cache, with no ROM reads.
 strcpy(g.sha1,"b4776b82a4c7915d0fadeaa27e013523f99dfd94");
 char dir[1024],cachefile[1200];snprintf(dir,sizeof(dir),"%s.cache",argv[1]);
 snprintf(cachefile,sizeof(cachefile),"%s/%s-v3.bin",dir,g.sha1);
 // A synthetic decoded palette also survives the asset cache round trip.
 size_t palette_size=pd_blob_sigs[PD_FR_CARD_PAL1].size;
 found[PD_FR_CARD_PAL1]=cp_malloc(palette_size);assert(found[PD_FR_CARD_PAL1]);
 memset(found[PD_FR_CARD_PAL1],0x5A,palette_size);foundRom=pd_rom_id(&g);
 assert(cp_cache_save(dir,&g));
 int before_section,before_dungeon;pd_map_pick(&g,0,4,4,&before_section,&before_dungeon);
 uint32_t first_pixel=art->page[0][0];
 cp_art_reset();cp_map_reset();R.preparing=false;
 assert(cp_cache_load(dir,&g));art=pd_map_art(&g);assert(art&&art->pages==1&&art->page[0][0]==first_pixel);
 const uint8_t*cached_palette=pd_romart_blob(&g,PD_FR_CARD_PAL1);assert(cached_palette);
 for(size_t i=0;i<palette_size;++i)assert(cached_palette[i]==0x5A);
 int section,dungeon;pd_map_pick(&g,0,4,4,&section,&dungeon);assert(section==before_section&&dungeon==before_dungeon);
 cp_art_reset();cp_map_reset();
 // Wrong ROM identity rejects the same cache payload.
 struct pd_game other=g;other.sha1[0]='a';assert(!cp_cache_load(dir,&other));
 // Checksum damage, unsupported version, and truncation all reject safely.
 f=fopen(cachefile,"r+b");assert(f);assert(!fseek(f,-1,SEEK_END));int last=fgetc(f);assert(last!=EOF);assert(!fseek(f,-1,SEEK_END));fputc(last^0x80,f);fclose(f);
 assert(!cp_cache_load(dir,&g));cp_art_reset();cp_map_reset();
 R.preparing=true;assert(pd_map_art(&g));assert(cp_cache_save(dir,&g));cp_art_reset();cp_map_reset();R.preparing=false;
 f=fopen(cachefile,"r+b");assert(f);fseek(f,4,SEEK_SET);fputc(99,f);fclose(f);assert(!cp_cache_load(dir,&g));cp_art_reset();cp_map_reset();
 R.preparing=true;assert(pd_map_art(&g));assert(cp_cache_save(dir,&g));cp_art_reset();cp_map_reset();R.preparing=false;
 assert(!truncate(cachefile,80));assert(!cp_cache_load(dir,&g));cp_art_reset();cp_map_reset();
 assert(!cp_cache_save("/nonexistent/companion-cache",&g));
 cp_adapter_close();assert(companion_memory()==0);
 puts("Cache: synthetic map/grid round trip, identity, checksum, version, truncation, write failure PASS");
 // Unsupported ROMs are refused by the production hash path without RAM writes.
 assert(!cp_adapter_open(argv[1]));assert(!companion_supported());assert(companion_memory()==0);
 companion_draw(ui_canvas);companion_touch(0,278,225);assert(companion_touch(2,278,225));cp_adapter_close();
 free(rom);free(a);free(b);puts("Adapter: segmented ROM, bounds, synthetic map, cleanup, unsupported ROM PASS");
}
