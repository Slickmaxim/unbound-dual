#include "alloc.h"
#include <string.h>
#include <stdint.h>
typedef union node node;
union node { struct { node *prev,*next; size_t size; } h; max_align_t align; };
static node *head;
static size_t used;
static int failed;
void *cp_malloc(size_t n) {
    if (!n) return NULL;
    if (used+sizeof(node)>CP_MEMORY_LIMIT || n > CP_MEMORY_LIMIT-used-sizeof(node) || n > SIZE_MAX-sizeof(node)) { failed=1; return NULL; }
    node *v=malloc(sizeof(*v)+n); if(!v){failed=1;return NULL;}
    v->h.prev=NULL;v->h.next=head;v->h.size=n;
    if(head)head->h.prev=v;head=v;used+=n+sizeof(node);
    return v+1;
}
void cp_free(void *p) {
    if(!p)return;node*v=(node*)p-1;
    if(v->h.prev)v->h.prev->h.next=v->h.next;else head=v->h.next;
    if(v->h.next)v->h.next->h.prev=v->h.prev;
    used-=v->h.size+sizeof(node);free(v);
}
void *cp_calloc(size_t n,size_t s){if(n&&s>SIZE_MAX/n)return NULL;void*p=cp_malloc(n*s);if(p)memset(p,0,n*s);return p;}
void *cp_realloc(void*p,size_t n){if(!p)return cp_malloc(n);if(!n){cp_free(p);return NULL;}node*v=(node*)p-1;void*q=cp_malloc(n);if(q){memcpy(q,p,n<v->h.size?n:v->h.size);cp_free(p);}return q;}
void cp_free_all(void){while(head)cp_free(head+1);failed=0;}
int cp_allocation_failed(void){return failed;}
size_t cp_memory_used(void){return used;}

void cp_allocation_clear_failure(void){failed=0;}
