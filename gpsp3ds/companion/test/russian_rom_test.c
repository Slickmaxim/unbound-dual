// Integration check for a user-supplied P.E. ROM and upstream RAM fixtures.
#include "../companion.c"
#include "../adapter.h"
#include "../vendor/pd_dex.h"
#include <assert.h>

unsigned char ewram[512 * 1024], iwram[64 * 1024];
unsigned char* memory_map_read[8192];
static uint16_t frame[320 * 240], english[320 * 240];
static unsigned char rom[32 * 1024 * 1024];

static void load_file(const char* path, void* out, size_t length) {
    FILE* file = fopen(path, "rb");
    assert(file && fread(out, 1, length, file) == length && fgetc(file) == EOF);
    fclose(file);
}

static void load_ram(const char* fixtures, const char* name) {
    char path[1200];
    snprintf(path, sizeof(path), "%s/%s/ewram.bin", fixtures, name);
    load_file(path, ewram, 262144);
    snprintf(path, sizeof(path), "%s/%s/iwram.bin", fixtures, name);
    load_file(path, iwram + 32768, 32768);
}

int main(int argc, char** argv) {
    assert(argc == 4);
    load_file(argv[1], rom, sizeof(rom));
    for (size_t i = 0; i < sizeof(rom) / 32768; i++)
        memory_map_read[4096 + i] = rom + i * 32768;

    const struct pd_glyph* builtin = pd_font_glyph(0x410);
    companion_cache_directory(argv[3]);
    load_ram(argv[2], "unbound");
    if (!cp_adapter_open(argv[1])) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }
    assert(S->game.language == 'R' && S->game.cfg->cyrillic);
    assert(S->item_profile && S->tm_profile);
    const struct pd_party_style* style = S->game.entry->partyStyle;
    const int party_blobs[] = {style->gfx, style->pal, style->slotMain, style->slotNoHp, style->slotEmpty,
                              style->ball, style->ballPal, style->status, style->statusPal, style->font};
    for (unsigned i = 0; i < sizeof(party_blobs) / sizeof(*party_blobs); i++)
        if (!pd_romart_blob(&S->game, party_blobs[i]))
            fprintf(stderr, "Missing party blob %d\n", party_blobs[i]);
    assert(pd_party_art_for(&S->game) && pd_romart_party_ready(&S->game));
    assert(pd_map_art(&S->game)->ok);

    char text[64];
    assert(!strcmp(pd_species_name(&S->game, 496, text, sizeof(text)), "Гибл"));
    assert(!strcmp(pd_species_name(&S->game, 29, text, sizeof(text)), "Нидоран♀"));
    assert(!strcmp(pd_move_name(&S->game, 33, text, sizeof(text)), "Таран"));
    assert(!strcmp(pd_item_name(&S->game, 13), "Зелье"));
    assert(!strcmp(pd_item_name(&S->game, 728), "Скала Хисуи"));
    assert(!strcmp(pd_mapsec_name(&S->game, 0x59), "город Беллин"));
    assert(!strcmp(pd_type_name(&S->game, 23), "Фея"));
    assert(pd_type_multiplier_pct(&S->game, 16, 23, PD_TYPE_NONE) == 0);
    assert(!strcmp(S->game.cfg->text->natures[0], "Выносливая"));
    assert(strstr(pd_item_description(&S->game, 13), "восстанавливает ОЗ"));
    struct pd_dex_entry dex;
    assert(pd_dex_entry(&S->game, 1, &dex));
    assert(!strcmp(dex.category, "Семя") && strlen(dex.description) > 20);

    for (unsigned cp = 0x401; cp <= 0x451; cp++) {
        if (cp != 0x401 && cp != 0x451 && (cp < 0x410 || cp > 0x44F)) continue;
        assert(pd_ru_codepoint(pd_ru_glyph_code(cp)) == cp);
        const struct pd_glyph* glyph = pd_font_glyph(cp);
        assert(glyph->codepoint == cp && glyph->advance > 0);
        unsigned pixels = 0;
        for (int y = 0; y < PD_FONT_HEIGHT; y++) pixels |= glyph->rows[y];
        assert(pixels);
    }

    // Only the fields that the translation changes are substituted into baseline RAM.
    size_t nickname = S->game.cfg->playerParty - 0x02000000 + S->game.cfg->partyLayout->nicknameOff;
    memcpy(ewram + nickname, rom + 0x166A98C + 496 * 11, 10);
    assert(companion_update(1));
    assert(S->snap.partyCount == 1 && S->snap.party[0].hp == 34);
    assert(!strcmp(S->snap.party[0].nickname, "Гибл"));
    S->ui.tab = PD_TAB_PARTY;
    companion_draw(frame);
    S->ui.summarySlot = 0;
    S->dirty = true;
    companion_draw(frame);
    S->ui.summarySlot = -1;
    const int tabs[] = {PD_TAB_PARTY, PD_TAB_BAG, PD_TAB_MAP, PD_TAB_CARD};
    for (size_t i = 0; i < sizeof(tabs) / sizeof(*tabs); ++i) {
        S->ui.tab = tabs[i];
        ud_language_set(UD_ENGLISH);
        companion_invalidate();
        companion_draw(english);
        ud_language_set(UD_RUSSIAN);
        companion_invalidate();
        companion_draw(frame);
        assert(memcmp(english, frame, sizeof(frame)));
    }
    assert(!strcmp(pd_item_name(&S->game, 13), "Зелье"));
    struct pd_card_info card;
    uint32_t pixels[PD_CARD_W * PD_CARD_H];
    assert(pd_card_info(&S->game, &S->snap, &card));
    assert(pd_card_render(&S->game, &card, false, true, true, pixels));
    assert(pd_card_render(&S->game, &card, true, true, true, pixels));

    load_ram(argv[2], "unbound_battle");
    S->ui.tab = PD_TAB_PARTY;
    S->dirty = true;
    assert(companion_update(10000));
    assert(S->snap.partyCount == 1 && S->snap.party[0].hp == 34);

    size_t memory = companion_memory();
    cp_adapter_close();
    assert(!companion_memory() && pd_font_glyph(0x410) == builtin);
    assert(cp_adapter_open(argv[1]));
    assert(!strcmp(companion_cache_status(), "hit"));
    assert(!strcmp(pd_item_name(&S->game, 13), "Зелье"));
    assert(pd_party_art_for(&S->game) && pd_font_glyph(0x410)->codepoint == 0x410);
    cp_adapter_close();
    assert(!companion_memory());
    printf("RU ROM: text, glyphs, Party, Bag, Map, Card, battle, item/TM guards and warm cache PASS (%zu bytes)\n", memory);
}
