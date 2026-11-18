#include "../cache.h"
#include "../progress.h"
#include "pd_romart.h"

#include <stdlib.h>
#include <string.h>

#include "pd_smol.h"
#include "pd_games.h"
#include "pd_unbound_es.h"

// --- finding blobs (RomArt.find) ---

// Both hosts (the 3DS's ARM11, the desktop) are little-endian: one unaligned
// load, where a byte-by-byte build was the scan's main cost.
static uint64_t u64le(const uint8_t* p) {
    uint64_t v;
    memcpy(&v, p, sizeof(v));
    return v;
}

static uint64_t fmix(uint64_t k) {
    k ^= k >> 33;
    k *= 0xFF51AFD7ED558CCDull;
    k ^= k >> 33;
    k *= 0xC4CEB9FE1A85EC53ull;
    k ^= k >> 33;
    return k;
}

static uint64_t head_hash(const uint8_t* p) {
    return fmix(u64le(p) ^ fmix(u64le(p + 8)));
}

static uint16_t prefilter(uint64_t a) {
    return (uint16_t) ((a * 0x9E3779B97F4A7C15ull) >> 48);
}

uint32_t pd_crc32(const uint8_t* data, size_t len) {
    static uint32_t table[256];
    if (!table[1]) {
        for (uint32_t n = 0; n < 256; n++) {
            uint32_t c = n;
            for (int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            table[n] = c;
        }
    }
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) c = table[(c ^ data[i]) & 0xFF] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

size_t pd_lz77(const uint8_t* src, size_t srcLen, uint8_t* dst, size_t dstLen) {
    if (srcLen < 4 || src[0] != 0x10) return 0;
    size_t size = src[1] | (size_t) src[2] << 8 | (size_t) src[3] << 16;
    if (size != dstLen) return 0;
    size_t in = 4, out = 0;
    while (out < size) {
        if (in >= srcLen) return 0;
        uint8_t flags = src[in++];
        for (int bit = 7; bit >= 0 && out < size; bit--) {
            if (flags >> bit & 1) {
                if (in + 1 >= srcLen) return 0;
                unsigned d = (unsigned) src[in] << 8 | src[in + 1];
                in += 2;
                size_t disp = (d & 0xFFF) + 1, len = (d >> 12) + 3;
                if (disp > out) return 0;
                for (size_t k = 0; k < len && out < size; k++, out++) dst[out] = dst[out - disp];
            } else {
                if (in >= srcLen) return 0;
                dst[out++] = src[in++];
            }
        }
    }
    return size;
}

// Decodes the blob that would start at off; true if its CRC matches.
static bool decode(const uint8_t* rom, size_t romSize, size_t off, const struct pd_blob_sig* sig, uint8_t* out) {
    if (sig->lz) {
        return pd_lz77(rom + off, romSize - off, out, sig->size) == sig->size && pd_crc32(out, sig->size) == sig->crc;
    }
    if (sig->smol) {
        size_t got = (rom[off] & 0xF) == 8 ? pd_smol_tilemap(rom + off, romSize - off, out, sig->size)
                                           : pd_smol(rom + off, romSize - off, out, sig->size);
        return got == sig->size && pd_crc32(out, sig->size) == sig->crc;
    }
    if (off + sig->size > romSize) return false;
    memcpy(out, rom + off, sig->size);
    return pd_crc32(out, sig->size) == sig->crc;
}

// Scans the ROM once for every wanted blob; each found one is decoded into
// out[i] (malloc'd, sig.size bytes), the rest left NULL.
static void find_blobs(const struct pd_game* g, const struct pd_blob_sig* const* sigs, int n, uint8_t** out) {
    uint64_t bits[1024] = {0};
    for (int k = 0; k < n; ++k) {
        bits[sigs[k]->pre >> 6] |= 1ull << (sigs[k]->pre & 63);
        out[k] = NULL;
    }
    uint8_t *chunk = malloc(65536 + 16);
    if (!chunk) return;
    int left = n;
    for (size_t base = 0; base < g->romSize && left; base += 65536) {
        cp_progress("Finding graphics",base,g->romSize);
        size_t count = g->romSize - base;
        if (count > 65536 + 16) count = 65536 + 16;
        if (!pd_rom_read(g, 0x08000000u + base, chunk, count)) break;
        for (size_t i = 0; i < 65536 && i + 16 <= count && left; ++i) {
            uint64_t v = u64le(chunk + i);
            uint16_t pre = prefilter(v);
            if (!(bits[pre >> 6] >> (pre & 63) & 1)) continue;
            uint64_t hash = head_hash(chunk + i);
            for (int k = 0; k < n; ++k) {
                const struct pd_blob_sig* sig = sigs[k];
                if (out[k] || sig->head != hash || base + i < sig->headOff) continue;
                size_t off = base + i - sig->headOff;
                size_t input = sig->lz ? sig->size + sig->size/8u + 16u : sig->smol ? sig->size * 2u + 4096u : sig->size;
                if (input > 128*1024) input = 128*1024;
                if (input > g->romSize - off) input = g->romSize - off;
                const uint8_t* bytes = pd_rom_at(g, 0x08000000u + off, input);
                uint8_t* result = malloc(sig->size);
                if (bytes && result && decode(bytes, input, 0, sig, result)) { out[k] = result; --left; }
                else free(result);
            }
        }
    }
    free(chunk);
}

void pd_romart_find(const struct pd_game* g, const struct pd_blob_sig* const* sigs, int n, uint8_t** out) {
    find_blobs(g, sigs, n, out);
}

// --- the cache: every blob of the game's family, found in one pass ---

static uint8_t* found[PD_BLOB_COUNT];
static unsigned foundRom;

void pd_romart_scan(const struct pd_game* g) {
    unsigned rom = pd_rom_id(g);
    if (foundRom == rom) return;
    for (int i = 0; i < PD_BLOB_COUNT; i++) {
        free(found[i]);
        found[i] = NULL;
    }
    foundRom = rom;
    int family = g->kind == PD_GAME_FIRERED ? PD_BLOB_FIRERED : g->kind == PD_GAME_EMERALD ? PD_BLOB_EMERALD : 0;
    if (!rom || !family) return;
    // The party's art: only what this game's slots and backdrop are made of, so a ROM that has it all stops the pass early.
    bool party[PD_BLOB_COUNT] = { [PD_FR_HOLD_GFX] = true, [PD_FR_HOLD_PAL] = true };
    const struct pd_game_entry* e = g->entry;
    const struct pd_party_style* st = e ? e->partyStyle : NULL;
    if (st) {
        const short blobs[] = { st->gfx, st->pal, st->slotMain, st->slotNoHp, st->slotEmpty, st->ball, st->ballPal, st->status, st->statusPal, st->font };
        for (size_t i = 0; i < sizeof(blobs) / sizeof(*blobs); i++)
            if (blobs[i] >= 0) party[blobs[i]] = true;
    }
    for (int i = 0; e && i < e->partyBackdropCount; i++) {
        party[e->partyBackdrops[i].gfx] = true;
        party[e->partyBackdrops[i].pal] = true;
        party[e->partyBackdrops[i].map] = true;
    }
    static const bool card[PD_BLOB_COUNT]={
        [PD_FR_CARD_GFX]=true,[PD_FR_CARD_FRONT]=true,[PD_FR_CARD_BACK]=true,[PD_FR_CARD_BG]=true,
        [PD_FR_CARD_PAL1]=true,[PD_UB_PIC_MALE]=true,[PD_UB_PIC_MALE_PAL]=true,
        [PD_UB_PIC_FEMALE]=true,[PD_UB_PIC_FEMALE_PAL]=true,[PD_FR_FONT_NORMAL]=true,
        [PD_FR_FONT_NORMAL_WIDTHS]=true,[PD_CARD_STAR_PAL]=true,
        [PD_FR_PLAYER_RED_GFX]=true,[PD_FR_PLAYER_LEAF_GFX]=true,[PD_FR_PLAYER_PAL]=true,
    };
    int wanted[PD_BLOB_COUNT], n = 0;
    const struct pd_blob_sig* sigs[PD_BLOB_COUNT];
    static uint8_t* decoded[PD_BLOB_COUNT];
    for (int i = 0; i < PD_BLOB_COUNT; i++) {
        if (party[i] || card[i]) {
            sigs[n] = &pd_blob_sigs[i];
            wanted[n++] = i;
        }
    }
    cp_progress("Finding graphics",0,0);
    find_blobs(g, sigs, n, decoded);
    for (int i = 0; i < n; i++) found[wanted[i]] = decoded[i];
    const struct pd_unbound_es_profile* profile = pd_unbound_es_profile(g->sha1);
    if (profile) {
        // Translated fonts/status labels can differ while their slot geometry
        // is unchanged. Recover only known resources from this exact ROM.
        const struct { enum pd_blob id; uint32_t offset, crc; bool lz; } local[] = {
            {PD_FR_FONT_NORMAL, 0x081F3100, profile->normalFontCRC, false},
            {PD_FR_FONT_NORMAL_WIDTHS, 0x081FB100, profile->normalWidthsCRC, false},
            {PD_FR_FONT_SMALL, 0x081EAF00, profile->smallFontCRC, false},
            {PD_UB_STATUS_GFX, 0x08B1E280, profile->statusGfxCRC, true}
        };
        for (unsigned i=0;i<sizeof(local)/sizeof(local[0]);i++) {
            enum pd_blob id=local[i].id;
            if (found[id]) continue;
            size_t size=pd_blob_sigs[id].size;
            uint8_t* bytes=malloc(size);
            bool ok=false;
            if (bytes && local[i].lz) {
                size_t input=size+size/8u+16u;
                const uint8_t* source=pd_rom_at(g,local[i].offset,input);
                ok=source && pd_lz77(source,input,bytes,size)==size;
            } else if (bytes) {
                ok=pd_rom_read(g,local[i].offset,bytes,size);
            }
            if (ok && pd_crc32(bytes,size)==local[i].crc) found[id]=bytes;
            else free(bytes);
        }
    }
}

const uint8_t* pd_romart_blob(const struct pd_game* g, enum pd_blob b) {
    pd_romart_scan(g);
    return b >= 0 && b < PD_BLOB_COUNT ? found[b] : NULL;
}


bool pd_romart_party_ready(const struct pd_game* g) {
    const struct pd_game_entry* entry=g->entry;
    const struct pd_party_style* st=entry ? entry->partyStyle : NULL;
    if (!st) return false;
    const short blobs[]={st->gfx,st->pal,st->slotMain,st->slotNoHp,st->slotEmpty,
                         st->ball,st->ballPal,st->status,st->statusPal,st->font};
    for (unsigned i=0;i<sizeof(blobs)/sizeof(blobs[0]);i++)
        if (blobs[i]>=0 && !pd_romart_blob(g,blobs[i])) return false;
    for (int i=0;i<entry->partyBackdropCount;i++) {
        const struct pd_party_backdrop* b=&entry->partyBackdrops[i];
        if (pd_romart_blob(g,b->gfx) && pd_romart_blob(g,b->pal) && pd_romart_blob(g,b->map)) return true;
    }
    return false;
}

void cp_art_reset(void) { for (int i=0;i<PD_BLOB_COUNT;++i) { free(found[i]); found[i]=NULL; } foundRom=0; }

bool cp_art_cache(struct cp_cache*c,const struct pd_game*g){
 if(cp_cache_u32(c,PD_BLOB_COUNT)!=PD_BLOB_COUNT)return false;
 for(int i=0;i<PD_BLOB_COUNT&&c->ok;++i){
  uint32_t n=cp_cache_u32(c,found[i]?pd_blob_sigs[i].size:0);
  if(n&&n!=pd_blob_sigs[i].size){cp_cache_reject(c);break;}
  if(!c->writing&&n){found[i]=malloc(n);if(!found[i]){cp_cache_reject(c);break;}}
  if(n)cp_cache_bytes(c,found[i],n);
 }
 if(c->ok)foundRom=pd_rom_id(g);
 return c->ok;
}
