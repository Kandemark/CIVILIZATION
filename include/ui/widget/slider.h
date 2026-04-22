#ifndef CIV_UI_WIDGET_SLIDER_H
#define CIV_UI_WIDGET_SLIDER_H

#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_widget_base_t base;
  float value;        /* 0.0 to 1.0 */
  float min, max;
  char  label[48];
  bool  dragging;
  bool  vertical;
  uint32_t color_track, color_fill, color_thumb;
} civ_slider_t;

civ_slider_t *civ_slider_create(const char *id, float x, float y,
                                float w, float h, float min, float max);
void civ_slider_destroy(civ_slider_t *s);
void civ_slider_set_value(civ_slider_t *s, float value);
float civ_slider_get_value(const civ_slider_t *s);
void civ_slider_update(civ_slider_t *s, civ_input_state_t *input, float dt);
void civ_slider_render(civ_slider_t *s, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
