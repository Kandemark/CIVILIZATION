/**
 * @file ui_common.h
 * @brief Common UI types, color constants, and utility functions
 */
#ifndef CIV_UI_COMMON_H
#define CIV_UI_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI Color Theme — legacy aliases, prefer display/theme.h for new code */
#define CIV_COLOR_BG_DARK       0x03050C
#define CIV_COLOR_BG_MEDIUM     0x070B1A
#define CIV_COLOR_BG_LIGHT      0x121828
#define CIV_COLOR_PRIMARY       0x00D4FF
#define CIV_COLOR_PRIMARY_DARK  0x004A7A
#define CIV_COLOR_ACCENT        0x00FFD2
#define CIV_COLOR_TEXT          0xF0F8FF
#define CIV_COLOR_TEXT_DIM      0xA0B4C8
#define CIV_COLOR_TEXT_DARK     0x4A5A6A
#define CIV_COLOR_SUCCESS       0x00FFB4
#define CIV_COLOR_WARNING       0xFFD400
#define CIV_COLOR_ERROR         0xFF4A6A
#define CIV_COLOR_GLOW          0x003A5A

typedef struct { int x, y, w, h; } civ_rect_t;

bool  civ_rect_contains(const civ_rect_t *rect, int x, int y);
float civ_clampf(float value, float min, float max);
int   civ_clampi(int value, int min, int max);
float civ_lerpf(float a, float b, float t);

#ifdef __cplusplus
}
#endif
#endif
