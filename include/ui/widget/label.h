#ifndef CIV_UI_WIDGET_LABEL_H
#define CIV_UI_WIDGET_LABEL_H

#include "widget_base.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CIV_LABEL_LEFT,
  CIV_LABEL_CENTER,
  CIV_LABEL_RIGHT
} civ_label_align_t;

typedef struct {
  civ_widget_base_t base;
  char text[256];
  uint32_t color;
  civ_label_align_t align;
  int font_size;
} civ_label_t;

civ_label_t *civ_label_create(const char *id, float x, float y, float w, float h,
                              const char *text);
void civ_label_destroy(civ_label_t *lbl);
void civ_label_set_text(civ_label_t *lbl, const char *text);
void civ_label_render(civ_label_t *lbl, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
