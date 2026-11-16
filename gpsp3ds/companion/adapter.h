#ifndef CP_ADAPTER_H
#define CP_ADAPTER_H
#include <stdbool.h>
bool cp_adapter_open(const char *rom_path);
void cp_adapter_close(void);
#endif
