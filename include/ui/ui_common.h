/**
 * @file ui_common.h
 * @brief Common UI types and utilities
 */

#ifndef CIV_UI_COMMON_H
#define CIV_UI_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI Color Theme */
/* UI Color Theme (Masterpiece 18.0: Stabilized Reality) */
#define CIV_COLOR_BG_DARK 0x03050C
#define CIV_COLOR_BG_MEDIUM 0x070B1A
#define CIV_COLOR_BG_LIGHT 0x121828
#define CIV_COLOR_PRIMARY 0x00D4FF
#define CIV_COLOR_PRIMARY_DARK 0x004A7A
#define CIV_COLOR_ACCENT 0x00FFD2
#define CIV_COLOR_TEXT 0xF0F8FF
#define CIV_COLOR_TEXT_DIM 0xA0B4C8
#define CIV_COLOR_TEXT_DARK 0x4A5A6A
#define CIV_COLOR_SUCCESS 0x00FFB4
#define CIV_COLOR_WARNING 0xFFD400
#define CIV_COLOR_ERROR 0xFF4A6A
#define CIV_COLOR_GLOW 0x003A5A

/**
 * Animation float value with smooth interpolation
 */
typedef struct {
  float current;
  float target;
  float speed; /**< Interpolation speed (0-1) */
} civ_anim_float_t;

/**
 * Update animated float value
 * @param anim Animation value to update
 */
void civ_anim_update(civ_anim_float_t *anim);

/**
 * Set animation target
 * @param anim Animation value
 * @param target New target value
 */
void civ_anim_set_target(civ_anim_float_t *anim, float target);

/**
 * Initialize animation value
 * @param anim Animation value
 * @param initial Initial/current value
 * @param speed Interpolation speed
 */
void civ_anim_init(civ_anim_float_t *anim, float initial, float speed);

/**
 * Rectangle structure
 */
typedef struct {
  int x, y, w, h;
} civ_rect_t;

/**
 * Check if point is inside rectangle
 * @param rect Rectangle
 * @param x Point X
 * @param y Point Y
 * @return True if point is inside
 */
bool civ_rect_contains(const civ_rect_t *rect, int x, int y);

/**
 * Clamp float value to range
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
float civ_clampf(float value, float min, float max);

/**
 * Clamp int value to range
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
int civ_clampi(int value, int min, int max);

/**
 * Linear interpolation
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor (0-1)
 * @return Interpolated value
 */
float civ_lerpf(float a, float b, float t);

#ifdef __cplusplus
}
#endif

#endif /* CIV_UI_COMMON_H */
