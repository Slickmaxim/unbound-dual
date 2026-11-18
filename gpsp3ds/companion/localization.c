#include "localization.h"
#include <stddef.h>
#include <string.h>
struct translation { const char *en, *es; };
#include "translations_es.inc"
static int language;
void ud_language_set(int spanish){language=spanish==1;}
int ud_language_get(void){return language;}
const char* ud_tr(const char* text){
 if(!language||!text||!*text)return text;
 size_t lo=0,hi=sizeof(translations)/sizeof(translations[0]);
 while(lo<hi){size_t mid=lo+(hi-lo)/2;int c=strcmp(text,translations[mid].en);
  if(!c)return translations[mid].es;if(c<0)hi=mid;else lo=mid+1;}
 return text;
}
