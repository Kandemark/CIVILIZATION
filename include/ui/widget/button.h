#ifndef CIV_UI_WIDGET_BUTTON_H
#define CIV_UI_WIDGET_BUTTON_H

#include "../../display/animation.h"
#include "../../display/theme.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct civ_widget_button {
  civ_widget_base_t base;
  char text[128];
  uint32_t color_bg;
  uint32_t color_hover;
  uint32_t color_press;
  uint32_t color_text;
  uint32_t color_border;
  int corner_radius;
  bool clicked;
  /* Animation */
  civ_tween_t hover_tween;
} civ_widget_button_t;

civ_widget_button_t *civ_widget_button_create(const char *id, float x, float y,
                                              float w, float h, const char *text);
void civ_widget_button_destroy(civ_widget_button_t *btn);
void civ_widget_button_set_text(civ_widget_button_t *btn, const char *text);
void civ_widget_button_set_enabled(civ_widget_button_t *btn, bool enabled);
void civ_widget_button_set_position(civ_widget_button_t *btn, float x, float y);
void civ_widget_button_update(civ_widget_button_t *btn, civ_input_state_t *input,
                              float dt);
void civ_widget_button_render(civ_widget_button_t *btn, SDL_Renderer *r,
                              civ_font_t *font);
bool civ_widget_button_was_clicked(civ_widget_button_t *btn);

#ifdef __cplusplus
}
#endif
#endif
