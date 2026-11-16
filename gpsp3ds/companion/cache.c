// Versioned, bounded little-endian data, never pointers or native structs.
#include "cache.h"
#include "vendor/pd_game.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define CACHE_VERSION 3
#define CACHE_LIMIT (2u*1024u*1024u)
void cp_cache_reject(struct cp_cache*c){c->ok=false;}
void cp_cache_bytes(struct cp_cache*c,void*p,size_t n){
 if(!c->ok)return;
 if(n>CACHE_LIMIT-c->bytes){c->ok=false;return;}
 if((c->writing?fwrite(p,1,n,c->file):fread(p,1,n,c->file))!=n){c->ok=false;return;}
 const unsigned char*b=p;for(size_t i=0;i<n;++i)c->hash=(c->hash^b[i])*16777619u;
 c->bytes+=n;
}
uint32_t cp_cache_u32(struct cp_cache*c,uint32_t x){
 unsigned char b[4]={(unsigned char)x,(unsigned char)(x>>8),(unsigned char)(x>>16),(unsigned char)(x>>24)};
 cp_cache_bytes(c,b,4);return (uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;
}
static bool transfer(FILE*f,bool writing,const struct pd_game*g){
 struct cp_cache c={f,writing,true,2166136261u,0};
 if(cp_cache_u32(&c,0x43425043)!=0x43425043||cp_cache_u32(&c,CACHE_VERSION)!=CACHE_VERSION)return false;
 char sha[40];memcpy(sha,g->sha1,40);cp_cache_bytes(&c,sha,40);
 if(memcmp(sha,g->sha1,40)||!c.ok)return false;
 if(!cp_art_cache(&c,g)||!cp_map_cache(&c,g)||!c.ok)return false;
 uint32_t hash=c.hash;
 if(cp_cache_u32(&c,hash)!=hash||!c.ok)return false;
 return writing||fgetc(f)==EOF;
}
static bool filename(char*out,size_t n,const char*dir,const struct pd_game*g){
 return dir&&dir[0]&&strlen(g->sha1)==40&&snprintf(out,n,"%s/%s-v%d.bin",dir,g->sha1,CACHE_VERSION)<(int)n;
}
bool cp_cache_load(const char*dir,const struct pd_game*g){
 char p[768];if(!filename(p,sizeof(p),dir,g))return false;
 FILE*f=fopen(p,"rb");if(!f)return false;
 bool ok=transfer(f,false,g);fclose(f);return ok;
}
bool cp_cache_save(const char*dir,const struct pd_game*g){
 char p[768],tmp[780];if(!filename(p,sizeof(p),dir,g))return false;
 mkdir(dir,0777);snprintf(tmp,sizeof(tmp),"%s.tmp",p);
 FILE*f=fopen(tmp,"wb");if(!f)return false;
 bool ok=transfer(f,true,g);if(fflush(f))ok=false;if(fclose(f))ok=false;
 if(ok&&rename(tmp,p)==0)return true;
 remove(tmp);return false;
}
