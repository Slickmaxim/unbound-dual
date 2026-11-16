#ifndef CP_CACHE_H
#define CP_CACHE_H
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
struct pd_game;
struct cp_cache {FILE *file;bool writing,ok;uint32_t hash;size_t bytes;};
uint32_t cp_cache_u32(struct cp_cache*,uint32_t);
void cp_cache_bytes(struct cp_cache*,void*,size_t);
void cp_cache_reject(struct cp_cache*);
bool cp_art_cache(struct cp_cache*,const struct pd_game*);
bool cp_map_cache(struct cp_cache*,const struct pd_game*);
bool cp_cache_load(const char *directory,const struct pd_game*);
bool cp_cache_save(const char *directory,const struct pd_game*);
#endif
