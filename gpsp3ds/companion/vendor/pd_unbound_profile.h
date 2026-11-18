// Exact translated ROM profiles for Unbound Dual. GPL-3.0.
#ifndef PD_UNBOUND_PROFILE_H
#define PD_UNBOUND_PROFILE_H
#include <stdint.h>
struct pd_text;
struct pd_unbound_profile {
    const char *sha1, *label;
    char language;
    const struct pd_text *text;
    uint32_t itemDescs, speciesInfo, speciesToNational, frontPics, palettes;
    uint32_t learnsets, monIconTable, itemIconTable;
    uint32_t normalFontCRC, normalWidthsCRC;
    uint32_t smallFontCRC, statusGfxCRC, partyGfxCRC;
};
// Unknown patches are deliberately not accepted by filename or game header.
const struct pd_unbound_profile* pd_unbound_profile(const char* sha1);
#endif
