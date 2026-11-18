#ifndef UD_LOCALIZATION_H
#define UD_LOCALIZATION_H

// Interface language is independent of the ROM language. Unknown keys retain English.
enum ud_language { UD_ENGLISH, UD_SPANISH, UD_RUSSIAN, UD_LANGUAGE_COUNT };
void ud_language_set(enum ud_language language);
enum ud_language ud_language_get(void);
void ud_language_set_code(const char* code);
const char* ud_language_code(void);
const char* ud_language_name(void);
const char* ud_tr(const char* english);
#endif
