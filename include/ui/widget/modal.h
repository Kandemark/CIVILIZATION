#ifndef CIV_UI_WIDGET_MODAL_H
#define CIV_UI_WIDGET_MODAL_H

#include "widget_base.h"
#include "../../display/animation.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_widget_base_t base;
  bool  is_open;
  char  title[128];
  char  message[512];
  bool  has_confirm;
  bool  has_cancel;
  bool  confirmed;
  bool  cancelled;
  civ_tween_t fade;
  float  overlay_alpha;
  uint32_t color_overlay, color_bg, color_border;
} civ_modal_t;

civ_modal_t *civ_modal_create(const char *id, const char *title,
                              const char *message);
void civ_modal_destroy(civ_modal_t *m);
void civ_modal_show(civ_modal_t *m);
void civ_modal_hide(civ_modal_t *m);
bool civ_modal_is_open(const civ_modal_t *m);
bool civ_modal_was_confirmed(const civ_modal_t *m);
bool civ_modal_was_cancelled(const civ_modal_t *m);
void civ_modal_update(civ_modal_t *m, civ_input_state_t *input, float dt,
                      int win_w, int win_h);
void civ_modal_render(civ_modal_t *m, SDL_Renderer *r, int win_w, int win_h);

#ifdef __cplusplus
}
#endif
#endif
