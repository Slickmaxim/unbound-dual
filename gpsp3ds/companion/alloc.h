#ifndef COMPANION_ALLOC_H
#define COMPANION_ALLOC_H
#include <stddef.h>
#include <stdlib.h>
void *cp_malloc(size_t n);
void *cp_calloc(size_t n, size_t size);
void *cp_realloc(void *p, size_t n);
void cp_free(void *p);
void cp_free_all(void);
size_t cp_memory_used(void);
int cp_allocation_failed(void);
void cp_allocation_clear_failure(void);
#define CP_MEMORY_LIMIT (2u * 1024u * 1024u)
#ifdef CP_REDIRECT_ALLOC
#define malloc cp_malloc
#define calloc cp_calloc
#define realloc cp_realloc
#define free cp_free
#endif
#endif
