// Russian Unbound ROM data and character encoding. GPL-3.0.
#ifndef PD_UNBOUND_RU_H
#define PD_UNBOUND_RU_H

#include <stdbool.h>
#include <stdint.h>

struct pd_game;

uint32_t pd_ru_codepoint(uint8_t byte);
// Zero when a code point does not use a Cyrillic slot.
uint8_t pd_ru_glyph_code(uint32_t codepoint);
bool pd_ru_load_text(struct pd_game* game);
void pd_ru_use_font(const struct pd_game* game);

#endif
