#ifndef UNBOUND_DUAL_COMPANION_H
#define UNBOUND_DUAL_COMPANION_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef bool (*cp_read_fn)(void*,uint32_t,void*,size_t);
// All interfaces are read-only. ROM span storage belongs to the adapter.
bool companion_load(const uint8_t header[192], size_t rom_size,
                    cp_read_fn read, const uint8_t* (*span)(void*,uint32_t,size_t), void *ctx);
void companion_close(void);
void companion_options(bool dark,bool smooth);
void companion_cache_directory(const char *directory);
const char *companion_cache_status(void);
bool companion_supported(void);
const char *companion_error(void);
// RAM polling only; the frontend presents an open Bag at a fixed 30 Hz.
bool companion_update(uint64_t now_us);
void companion_draw(uint16_t *rgb565);
// Touch phases: 0 down, 1 held, 2 release. Return true for native MENU.
bool companion_touch(int phase,int x,int y);
void companion_invalidate(void);
bool companion_bag_open(void);
unsigned companion_interval_ms(void);
void companion_feedback(unsigned elapsed_us);
size_t companion_memory(void);
#endif
