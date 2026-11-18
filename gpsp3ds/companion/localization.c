#include "localization.h"
#include <stddef.h>
#include <string.h>

struct translation { const char *en, *localized; };
#include "translations_es.inc"
#include "translations_ru.inc"

static const struct language {
    const char *code, *name;
    const struct translation* translations;
    size_t count;
} languages[UD_LANGUAGE_COUNT] = {
    [UD_ENGLISH] = {"en", "English", NULL, 0},
    [UD_SPANISH] = {"es", "Español", translations_es, sizeof translations_es / sizeof *translations_es},
    [UD_RUSSIAN] = {"ru", "Русский", translations_ru, sizeof translations_ru / sizeof *translations_ru},
};

static enum ud_language language;

void ud_language_set(enum ud_language value) {
    language = (unsigned)value < UD_LANGUAGE_COUNT ? value : UD_ENGLISH;
}

enum ud_language ud_language_get(void) { return language; }
const char* ud_language_code(void) { return languages[language].code; }
const char* ud_language_name(void) { return languages[language].name; }

void ud_language_set_code(const char* code) {
    for (int i = 0; code && i < UD_LANGUAGE_COUNT; ++i) {
        if (!strcmp(code, languages[i].code)) {
            ud_language_set(i);
            return;
        }
    }
    ud_language_set(UD_ENGLISH);
}

const char* ud_tr(const char* text) {
    if (!text || !*text) return text;
    const struct language* selected = &languages[language];
    size_t lo = 0, hi = selected->count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const struct translation* entry = &selected->translations[mid];
        int cmp = strcmp(text, entry->en);
        if (!cmp) return entry->localized;
        if (cmp < 0) hi = mid;
        else lo = mid + 1;
    }
    return text;
}
