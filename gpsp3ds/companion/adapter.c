#include "adapter.h"
#include "companion.h"
#include "alloc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
extern unsigned char ewram[512*1024],iwram[64*1024];
extern unsigned char *memory_map_read[8192];
static struct { FILE *file; size_t size; uint8_t header[192]; bool preparing; uint8_t*scratch[2]; unsigned next; } R;
static char rom_path_saved[700];
static char trusted_sha[41];
static bool trusted_identity;
static uint32_t sample_hash(const uint8_t*p,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static bool identity_stamp(bool write_stamp,size_t size){
 struct stat st; if(stat(rom_path_saved,&st))return false; char path[800];snprintf(path,sizeof(path),"sdmc:/3ds/gpsp3d/companion-cache/rom-id.bin");
 FILE*f=fopen(rom_path_saved,"rb");if(!f)return false;uint8_t b[4096];size_t n=fread(b,1,sizeof(b),f);uint32_t first=sample_hash(b,n);if(fseek(f,size>sizeof(b)?(long)(size-sizeof(b)):0,SEEK_SET)){fclose(f);return false;}n=fread(b,1,sizeof(b),f);uint32_t last=sample_hash(b,n);fclose(f);
 struct {uint32_t magic,version,size,mtime,first,last;char sha[41];} v={0x49425043,1,(uint32_t)size,(uint32_t)st.st_mtime,first,last,{0}};
 if(write_stamp){memcpy(v.sha,trusted_sha,41);mkdir("sdmc:/3ds/gpsp3d/companion-cache",0777);f=fopen(path,"wb");if(!f)return false;bool ok=fwrite(&v,1,sizeof(v),f)==sizeof(v);fclose(f);return ok;}
 f=fopen(path,"rb");if(!f)return false;struct {uint32_t magic,version,size,mtime,first,last;char sha[41];} old;bool ok=fread(&old,1,sizeof(old),f)==sizeof(old)&&fgetc(f)==EOF;fclose(f);if(!ok||old.magic!=v.magic||old.version!=1||old.size!=v.size||old.mtime!=v.mtime||old.first!=v.first||old.last!=v.last||old.sha[40])return false;memcpy(trusted_sha,old.sha,41);return true;
}
bool cp_identity_sha(size_t size,char out[41]){if(!trusted_identity)trusted_identity=identity_stamp(false,size);if(trusted_identity){memcpy(out,trusted_sha,41);return true;}return false;}
void cp_identity_commit(const char sha[41]){memcpy(trusted_sha,sha,41);trusted_identity=true;identity_stamp(true,R.size);}
static bool read_bus(void*ctx,uint32_t a,void*out,size_t n){
    (void)ctx;
    if(a>=0x02000000&&a<=0x02040000&&n<=0x02040000-a){memcpy(out,ewram+a-0x02000000,n);return true;}
    if(a>=0x03000000&&a<=0x03008000&&n<=0x03008000-a){memcpy(out,iwram+32768+a-0x03000000,n);return true;}
    if(a<0x08000000||a-0x08000000>R.size||n>R.size-(a-0x08000000))return false;
    if(R.preparing)return R.file&&!fseek(R.file,(long)(a-0x08000000),SEEK_SET)&&fread(out,1,n,R.file)==n;
    // Read resident pages without invoking gpSP's paging/eviction machinery.
    uint8_t*d=out;
    while(n){unsigned off=a&32767;size_t part=32768-off;if(part>n)part=n;
        unsigned char*p=memory_map_read[a>>15];if(!p)return false;
        memcpy(d,p+off,part);d+=part;a+=part;n-=part;
    }return true;
}
static const uint8_t*span(void*ctx,uint32_t a,size_t n){
    if(a<0x08000000||a-0x08000000>=R.size||n>R.size-(a-0x08000000))return NULL;
    // Several upstream text/LZ readers inspect beyond a four-byte header.
    size_t available=R.size-(a-0x08000000);
    size_t want=n==4?128*1024:n==1?512:n;
    if(want>available)want=available;
    if(want<n||want>128*1024)return NULL;
    if(!R.preparing){
        uint8_t*p=memory_map_read[a>>15];
        if(p){p+=a&32767;bool contiguous=true;
            for(size_t k=32768-(a&32767);k<want;k+=32768)if(memory_map_read[(a+k)>>15]!=p+k){contiguous=false;break;}
            if(contiguous)return p;
        }
    }
    // Two stable scratch spans cover boundary-crossing compressed records.
    unsigned at=R.next++%2;
    if(!R.scratch[at])R.scratch[at]=cp_malloc(128*1024);
    if(!R.scratch[at]||!read_bus(ctx,a,R.scratch[at],want))return NULL;
    return R.scratch[at];
}
void cp_adapter_close(void){if(R.file)fclose(R.file);companion_close();memset(&R,0,sizeof(R));}
bool cp_adapter_open(const char*path){
    cp_adapter_close();snprintf(rom_path_saved,sizeof(rom_path_saved),"%s",path);trusted_identity=false;R.file=fopen(path,"rb");if(!R.file)return false;
    if(fseek(R.file,0,SEEK_END)||ftell(R.file)<=0){cp_adapter_close();return false;}
    R.size=(size_t)ftell(R.file);rewind(R.file);
    if(R.size>32u*1024u*1024u||fread(R.header,1,sizeof(R.header),R.file)!=sizeof(R.header)){cp_adapter_close();return false;}
    R.preparing=true;
    bool ok=companion_load(R.header,R.size,read_bus,span,NULL);
    R.preparing=false;fclose(R.file);R.file=NULL;
    return ok;
}
