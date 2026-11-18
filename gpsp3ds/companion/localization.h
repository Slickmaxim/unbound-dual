#ifndef UD_LOCALIZATION_H
#define UD_LOCALIZATION_H
// Interface language is independent of the ROM language. Unknown keys retain English.
void ud_language_set(int spanish);
int ud_language_get(void);
const char* ud_tr(const char* english);
#endif
