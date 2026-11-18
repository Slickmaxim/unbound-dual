// Exact Spanish ROM profiles for Unbound Dual. GPL-3.0.
#ifndef PD_UNBOUND_ES_H
#define PD_UNBOUND_ES_H
#include <stdint.h>
struct pd_text;
struct pd_unbound_es_profile {
    const char *sha1, *label;
    const struct pd_text *text;
    uint32_t itemDescs, speciesInfo, speciesToNational, frontPics, palettes;
    uint32_t learnsets, monIconTable, itemIconTable;
    uint32_t normalFontCRC, normalWidthsCRC;
    uint32_t smallFontCRC, statusGfxCRC;
};
// Unknown patches are deliberately not accepted by filename or game header.
const struct pd_unbound_es_profile* pd_unbound_es_profile(const char* sha1);
#endif
