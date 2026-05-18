/**
 * @file tooltip.h
 * @brief Hover tooltip with nested data chains (CK3/EU4 style)
 *
 * Tooltips auto-position to avoid screen edges. They can chain:
 * hovering a stat inside a tooltip can spawn a sub-tooltip explaining
 * the components of that value (e.g., "Tax Income: 24.5 Gold" →
 * "Base: 15.0 | Modifiers: +3.0 Market, +2.5 Governor, +4.0 Policy").
 */
#ifndef CIV_UI_WIDGET_TOOLTIP_H
#define CIV_UI_WIDGET_TOOLTIP_H

#include "../../display/animation.h"
#include "../../display/theme.h"
#include "../../engine/font.h"
#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_TOOLTIP_LINES_MAX 12
#define CIV_TOOLTIP_LINE_LEN  96

typedef struct {
  char     text[CIV_TOOLTIP_LINE_LEN];
  uint32_t color;
  bool     is_header;
  bool     separator_before;
} civ_tooltip_line_t;

typedef struct {
  civ_widget_base_t base;
  civ_tooltip_line_t lines[CIV_TOOLTIP_LINES_MAX];
  int   line_count;
  float pad;
  float line_height;
  int   font_size;

  /* Animation */
  civ_tween_t fade;
  float  target_alpha;
  bool   pending_show;

  /* Cursor tracking */
  float cursor_x, cursor_y;
  float offset_x, offset_y;  /* offset from cursor */

  /* Content buffer for formatting */
  char format_buf[512];
} civ_tooltip_t;

civ_tooltip_t *civ_tooltip_create(const char *id);
void civ_tooltip_destroy(civ_tooltip_t *t);

/* Clear all lines and reset */
void civ_tooltip_clear(civ_tooltip_t *t);

/* Add formatted lines — returns t for chaining */
civ_tooltip_t *civ_tooltip_add_header(civ_tooltip_t *t, const char *text,
                                      uint32_t color);
civ_tooltip_t *civ_tooltip_add_line(civ_tooltip_t *t, const char *text,
                                    uint32_t color);
civ_tooltip_t *civ_tooltip_add_separator(civ_tooltip_t *t);
civ_tooltip_t *civ_tooltip_add_key_value(civ_tooltip_t *t, const char *key,
                                          const char *value, uint32_t key_color,
                                          uint32_t val_color);
civ_tooltip_t *civ_tooltip_add_bar(civ_tooltip_t *t, const char *label,
                                    float value, uint32_t bar_color);

/* Called every frame by the scene */
void civ_tooltip_update(civ_tooltip_t *t, civ_input_state_t *input, float dt,
                        int win_w, int win_h);
void civ_tooltip_render(civ_tooltip_t *t, SDL_Renderer *r, civ_font_t *font);

/* Request show at cursor position. Auto-hides when the tracked target
 * is no longer hovered (checked externally by caller). */
void civ_tooltip_show(civ_tooltip_t *t, float cursor_x, float cursor_y);
void civ_tooltip_hide(civ_tooltip_t *t);

#ifdef __cplusplus
}
#endif
#endif
