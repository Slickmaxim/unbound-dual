// Russian Unbound ROM data. GPL-3.0.
// Table layout and glyph slots verified against the P.E. translation:
// https://github.com/PEdeveloper/pokemon-unbound-ru
#include "pd_unbound_ru.h"
#include "pd_font.h"
#include "pd_game.h"
#include "pd_romart.h"
#include "pd_snapshot.h"
#include "pd_tables.h"

#include <stdlib.h>
#include <string.h>

static const uint8_t extra_upper[] = { 1, 2, 3, 4, 5, 6, 7 };
static const uint8_t extra_lower[] = { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x20 };

static uint8_t glyph_slot(bool lower, int index) {
    if (index < 26) return (uint8_t) ((lower ? 0xD5 : 0xBB) + index);
    return (lower ? extra_lower : extra_upper)[index - 26];
}

static uint32_t letter(bool lower, int index) {
    if (index == 6) return lower ? 0x451 : 0x401; // ё/Ё follows е/Е.
    return (lower ? 0x430 : 0x410) + index - (index > 6);
}

uint32_t pd_ru_codepoint(uint8_t byte) {
    if (byte >= 0xBB && byte <= 0xEE)
        return byte < 0xD5 ? letter(false, byte - 0xBB) : letter(true, byte - 0xD5);
    for (unsigned i = 0; i < sizeof(extra_lower); ++i) {
        if (byte == extra_upper[i]) return letter(false, 26 + i);
        if (byte == extra_lower[i]) return letter(true, 26 + i);
    }
    return 0;
}

uint8_t pd_ru_glyph_code(uint32_t cp) {
    if (cp == 0x401 || cp == 0x451) return glyph_slot(cp == 0x451, 6);
    if (cp < 0x410 || cp > 0x44F) return 0;
    int index = (cp - 0x410) % 32;
    return glyph_slot(cp >= 0x430, index + (index >= 6));
}

enum {
    SPECIES_COUNT = 1294, ITEM_COUNT = 729, MOVE_COUNT = 923,
    MAP_FIRST = 0x58, MAP_COUNT = 109, TYPE_COUNT = 24, NATURE_COUNT = 25, TYPE_PAIRS = 131
};

struct ru_text {
    struct pd_config cfg;
    struct pd_text text;
    const char* species[SPECIES_COUNT];
    const char* items[ITEM_COUNT];
    const char* maps[MAP_FIRST + MAP_COUNT];
    const char* types[TYPE_COUNT];
    const char* natures[NATURE_COUNT];
    struct pd_move_info moves[MOVE_COUNT];
    struct pd_type_pair chart[TYPE_PAIRS];
    uint8_t speciesTypes[SPECIES_COUNT][2];
    size_t namesUsed;
    char names[56 * 1024];
};

static uint32_t read_pointer(const uint8_t* data) {
    return data[0] | (uint32_t)data[1] << 8 | (uint32_t)data[2] << 16 | (uint32_t)data[3] << 24;
}

static const char* store_name(const struct pd_game* g, struct ru_text* ru, const uint8_t* data, size_t length) {
    char text[48 * 3 + 1];
    pd_gen3_text_for(g->cfg, data, length, text, sizeof(text));
    size_t bytes = strlen(text) + 1;
    if (bytes > sizeof(ru->names) - ru->namesUsed) return NULL;
    char* stored = ru->names + ru->namesUsed;
    memcpy(stored, text, bytes);
    ru->namesUsed += bytes;
    return stored;
}

static bool load_pointer_table(const struct pd_game* g, struct ru_text* ru, uint32_t address,
                               const char** names, int count, size_t length) {
    uint8_t pointers[MAP_COUNT * 4];
    if (!pd_rom_read(g, address, pointers, (size_t)count * 4)) return false;
    for (int i = 0; i < count; ++i) {
        const uint8_t* data = pd_rom_at(g, read_pointer(pointers + i * 4), length);
        if (!data) return false;
        names[i] = store_name(g, ru, data, length);
        if (!names[i]) return false;
    }
    return true;
}

static bool load_items(const struct pd_game* g, struct ru_text* ru) {
    const struct pd_item_descs* items = g->cfg->itemDescs;
    uint8_t records[16][44];
    for (int first = 1; first < ITEM_COUNT; first += 16) {
        int count = ITEM_COUNT - first < 16 ? ITEM_COUNT - first : 16;
        if (!pd_rom_read(g, items->base + 44u * first, records, (size_t)count * 44)) return false;
        for (int i = 0; i < count; ++i) {
            const uint8_t* record = records[i];
            unsigned id = record[14] | (unsigned)record[15] << 8;
            if (!id || id >= ITEM_COUNT || ru->items[id]) continue;
            uint32_t pointer = read_pointer(record);
            bool indirect = pointer >= 0x08000000 && pointer - 0x08000000 < g->romSize;
            size_t length = indirect ? 32 : 14;
            const uint8_t* data = indirect ? pd_rom_at(g, pointer, length) : record;
            if (!data) return false;
            if (data[0] == 0xFF || (length >= 4 && !memcmp(data, "\xAC\xAC\xAC\xAC", 4))) continue;
            ru->items[id] = store_name(g, ru, data, length);
            if (!ru->items[id]) return false;
        }
    }
    return true;
}

bool pd_ru_load_text(struct pd_game* g) {
    struct ru_text* ru = calloc(1, sizeof(*ru));
    if (!ru) return false;
    const uint8_t* data = pd_rom_at(g, 0x0966A98C, SPECIES_COUNT * 11);
    if (!data) goto fail;
    for (int i = 1; i < SPECIES_COUNT; ++i) {
        ru->species[i] = store_name(g, ru, data + i * 11, 11);
        if (!ru->species[i]) goto fail;
    }
    data = pd_rom_at(g, 0x08A40A10, MOVE_COUNT * 13);
    if (!data) goto fail;
    for (int i = 1; i < MOVE_COUNT; ++i) {
        const char* name = store_name(g, ru, data + i * 13, 13);
        if (!name) goto fail;
        ru->moves[i] = g->cfg->text->moves[i];
        if (*name) ru->moves[i].name = name;
    }
    data = pd_rom_at(g, 0x08A4EAD4, TYPE_COUNT * 7);
    if (!data) goto fail;
    for (int i = 0; i < TYPE_COUNT; ++i) {
        ru->types[i] = store_name(g, ru, data + i * 7, 7);
        if (!ru->types[i]) goto fail;
    }
    data = pd_rom_at(g, g->cfg->speciesInfo, SPECIES_COUNT * 28);
    if (!data) goto fail;
    for (int i = 1; i < SPECIES_COUNT; ++i) {
        ru->speciesTypes[i][0] = data[i * 28 + 6];
        ru->speciesTypes[i][1] = data[i * 28 + 7];
    }
    // CFRU inserts a marker before the immunities ignored by Foresight.
    uint8_t chart[(TYPE_PAIRS + 1) * 3 + 1];
    if (!pd_rom_read(g, 0x089619C0, chart, sizeof(chart))) goto fail;
    int pairs = 0;
    for (size_t i = 0; i + 2 < sizeof(chart); i += 3) {
        if (chart[i] == 0xFE) continue;
        if (pairs == TYPE_PAIRS || chart[i] >= TYPE_COUNT || chart[i + 1] >= TYPE_COUNT) goto fail;
        ru->chart[pairs++] = (struct pd_type_pair){chart[i], chart[i + 1], chart[i + 2] * 10};
    }
    if (pairs != TYPE_PAIRS || chart[sizeof(chart) - 1] != 0xFF) goto fail;
    if (!load_items(g, ru) ||
        !load_pointer_table(g, ru, 0x083F1CAC, ru->maps + MAP_FIRST, MAP_COUNT, 48) ||
        !load_pointer_table(g, ru, 0x08463E60, ru->natures, NATURE_COUNT, 24))
        goto fail;
    ru->text = *g->cfg->text;
    ru->text.species = ru->species;
    ru->text.items = ru->items;
    ru->text.moves = ru->moves;
    ru->text.typeNames = ru->types;
    ru->text.typeNamesCount = TYPE_COUNT;
    ru->text.typeChart = ru->chart;
    ru->text.typeChartCount = TYPE_PAIRS;
    ru->text.speciesTypes = ru->speciesTypes;
    ru->text.speciesTypesCount = SPECIES_COUNT;
    ru->text.mapsecs = ru->maps;
    ru->text.mapsecsCount = MAP_FIRST + MAP_COUNT;
    ru->text.mapsecsRom = false;
    ru->text.natures = ru->natures;
    ru->cfg = *g->cfg;
    ru->cfg.text = &ru->text;
    g->cfg = &ru->cfg;
    return true;

fail:
    free(ru);
    return false;
}

void pd_ru_use_font(const struct pd_game* g) {
    static struct pd_glyph glyphs[66];
    const uint8_t* font = pd_romart_blob(g, PD_FR_FONT_NORMAL);
    const uint8_t* widths = pd_romart_blob(g, PD_FR_FONT_NORMAL_WIDTHS);
    if (!font || !widths) return;
    for (int i = 0; i < 66; ++i) {
        struct pd_glyph* glyph = &glyphs[i];
        uint8_t code = glyph_slot(i >= 33, i % 33);
        glyph->codepoint = (uint16_t)letter(i >= 33, i % 33);
        glyph->advance = widths[code];
        for (int y = 0; y < PD_FONT_HEIGHT; ++y) {
            uint16_t pixels = 0;
            for (int x = 0; x < 16; ++x) {
                int offset = code * 64 + ((y / 8) * 2 + x / 8) * 16 + (y % 8) * 2;
                unsigned row = font[offset] | (unsigned)font[offset + 1] << 8;
                if ((row >> (14 - 2 * (x % 8)) & 3) == 1) pixels |= 1u << x;
            }
            glyph->rows[y] = pixels;
        }
    }
    pd_font_set(glyphs, 66);
}
