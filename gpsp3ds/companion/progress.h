#ifndef CP_PROGRESS_H
#define CP_PROGRESS_H
#include <stddef.h>
typedef void (*cp_progress_fn)(const char*,unsigned,void*);
void cp_progress_set(cp_progress_fn fn,void *ctx);
void cp_progress(const char *stage,size_t done,size_t total);
#endif
