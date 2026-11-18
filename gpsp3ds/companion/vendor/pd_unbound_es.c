// Unbound Dual Spanish adaptations; GPL-3.0.
// Runtime table pointers were verified against the respective ROM's code.
#include "pd_unbound_es.h"
#include <stddef.h>
#include <string.h>
extern const struct pd_text pd_text_unbound_es;
extern const struct pd_text pd_text_unbound_es_sinza;
extern const struct pd_text pd_text_unbound_es_2111;
static const struct pd_unbound_es_profile profiles[] = {
    {
        "d5b312465c0a95bcb6428bd61cc170994a2a65fc", "Unbound ES v1.08",
        &pd_text_unbound_es,
        0x095FBC90, 0x0963BC90, 0x09FF7C70, 0x09FE6C64, 0x09FEBE84,
        0x09FF10A4, 0x09FF252C, 0x096040BC, 0xF95367FD, 0x51A98C02
    },
    {
        "117449bd7822fad0db293a4f7e364787e4f8543a", "Unbound ES v1.08 sinza",
        &pd_text_unbound_es_sinza,
        0x08876200, 0x099E0C9C, 0x09A41FEC, 0x09A1D5B4, 0x09A32BDC,
        0x09A2457C, 0x09A217FC, 0x0825A554, 0xF95367FD, 0x51A98C02
    },
    {
        "1104054a3061b35b85b04757fbca5a3686cca60c", "Unbound ES 2.1.1.1",
        &pd_text_unbound_es_2111,
        0x08876200, 0x099E0C9C, 0x09A41FEC, 0x09A1D5B4, 0x09A32BDC,
        0x09A2457C, 0x09A217FC, 0x0825A554, 0x4EEA4916, 0xC3CEC263
    }
};
const struct pd_unbound_es_profile* pd_unbound_es_profile(const char* sha1) {
    if (!sha1) return NULL;
    for (unsigned i=0;i<sizeof(profiles)/sizeof(profiles[0]);i++)
        if (!strcmp(sha1,profiles[i].sha1)) return &profiles[i];
    return NULL;
}
