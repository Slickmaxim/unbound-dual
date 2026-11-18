#include "../localization.h"
#include "../vendor/pd_canvas.h"
#include "../vendor/pd_font.h"
#include "../../source/ui_screens.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char* codes[] = {"en", "es", "ru"};
    for (int language = 0; language < UD_LANGUAGE_COUNT; ++language) {
        ud_language_set(language);
        assert(!strcmp(ud_language_code(), codes[language]));
        ud_language_set_code(ud_language_code());
        assert((int)ud_language_get() == language);
    }
    ud_language_set_code("invalid");
    assert(ud_language_get() == UD_ENGLISH);
    ud_language_set_code("es");
    assert(!strcmp(ud_tr("BAG"), "MOCHILA"));
    ud_language_set_code("ru");
    assert(!strcmp(ud_language_name(), "Русский"));
    assert(!strcmp(ud_tr("Interface language"), "Язык интерфейса"));
    assert(!strcmp(ud_tr("BAG"), "СУМКА"));
    assert(!strcmp(ud_tr("Gible"), "Gible"));

    const ui_font* fonts[] = {&ui_font_small, &ui_font_body, &ui_font_bold, &ui_font_title, &ui_font_big};
    for (unsigned cp = 0x401; cp <= 0x451; cp++) {
        if (cp != 0x401 && cp != 0x451 && (cp < 0x410 || cp > 0x44F)) continue;
        for (size_t i = 0; i < sizeof(fonts) / sizeof(*fonts); ++i) {
            const ui_glyph* glyph = ui_glyph_of(fonts[i], cp);
            assert(glyph != ui_glyph_of(fonts[i], '?') && glyph->adv > 0);
        }
        const struct pd_glyph* glyph = pd_font_glyph(cp);
        assert(glyph->codepoint == cp && glyph->advance > 0);
        unsigned ink = 0;
        for (int y = 0; y < PD_FONT_HEIGHT; ++y) ink |= glyph->rows[y];
        assert(ink);
    }

    const char* tabs[] = {"PARTY", "BAG", "MAP", "CARD", "MENU"};
    for (size_t i = 0; i < sizeof(tabs) / sizeof(*tabs); ++i)
        assert(pd_text_width(tabs[i]) <= 58);

    const char* menu_tabs[] = {"Game", "Display", "3D", "Speed", "Keys", "System"};
    int width = 0;
    for (size_t i = 0; i < sizeof(menu_tabs) / sizeof(*menu_tabs); ++i)
        width += ui_text_w(&ui_font_bold, menu_tabs[i]);
    assert(width < UI_W);
    assert(ui_text_w(&ui_font_bold, ud_language_name()) <= 80);

    const ui_row row = {.kind = ROW_CHOICE, .label = "Interface language", .value = ud_language_name()};
    const ui_menu menu = {.tabs = menu_tabs, .ntabs = 6, .tab = 1, .rows = &row, .nrows = 1};
    scr_menu(&menu);
    static uint16_t russian[UI_W * UI_H];
    memcpy(russian, ui_canvas, sizeof(russian));
    assert(scr_menu_hit(&menu, 298, MENU_LIST_Y + 14).zone == ZONE_INC);
    ud_language_set(UD_ENGLISH);
    scr_menu(&menu);
    assert(memcmp(russian, ui_canvas, sizeof(russian)));
    puts("Localization: English/Spanish/Russian config keys, Cyrillic glyphs and menu rendering PASS");
}
