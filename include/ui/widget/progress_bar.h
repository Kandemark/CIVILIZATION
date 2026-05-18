#ifndef CIV_UI_WIDGET_PROGRESS_BAR_H
#define CIV_UI_WIDGET_PROGRESS_BAR_H

#include "widget_base.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_widget_base_t base;
  float value;       /* 0.0 to 1.0 */
  uint32_t color_fg;
  uint32_t color_bg;
  char label[64];
  bool vertical;
} civ_progress_bar_t;

civ_progress_bar_t *civ_progress_bar_create(const char *id, float x, float y,
                                            float w, float h);
void civ_progress_bar_destroy(civ_progress_bar_t *pb);
void civ_progress_bar_set_value(civ_progress_bar_t *pb, float value);
void civ_progress_bar_render(civ_progress_bar_t *pb, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
