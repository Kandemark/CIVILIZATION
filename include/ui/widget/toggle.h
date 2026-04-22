#ifndef CIV_UI_WIDGET_TOGGLE_H
#define CIV_UI_WIDGET_TOGGLE_H

#include "widget_base.h"
#include "../../display/animation.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_widget_base_t base;
  bool  value;
  char  label[64];
  civ_tween_t thumb_tween;
  uint32_t color_on, color_off, color_thumb;
} civ_toggle_t;

civ_toggle_t *civ_toggle_create(const char *id, float x, float y,
                                float w, float h, const char *label);
void civ_toggle_destroy(civ_toggle_t *t);
void civ_toggle_set_value(civ_toggle_t *t, bool value);
bool civ_toggle_get_value(const civ_toggle_t *t);
void civ_toggle_update(civ_toggle_t *t, civ_input_state_t *input, float dt);
void civ_toggle_render(civ_toggle_t *t, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
