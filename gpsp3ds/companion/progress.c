#include "progress.h"
static cp_progress_fn callback;
static void *context;
static const char *last_stage;
static unsigned last_percent;
void cp_progress_set(cp_progress_fn fn,void *ctx){callback=fn;context=ctx;last_stage=0;last_percent=101;}
void cp_progress(const char *stage,size_t done,size_t total){
 unsigned percent=total?(unsigned)((unsigned long long)done*100/total):101;
 if(callback&&(stage!=last_stage||percent!=last_percent)){last_stage=stage;last_percent=percent;callback(stage,percent,context);}
}
