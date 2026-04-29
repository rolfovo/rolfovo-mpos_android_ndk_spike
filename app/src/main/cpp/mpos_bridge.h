#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mpos_bridge_set_title(const char *text);
void mpos_bridge_set_subtitle(const char *text);
void mpos_bridge_set_status(const char *text);
uint32_t mpos_bridge_click_count(void);

#ifdef __cplusplus
}
#endif
