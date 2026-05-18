/**
 * @file panel.h
 * @brief Draggable, closable, animated panel container widget
 */
#ifndef CIV_UI_WIDGET_PANEL_H
#define CIV_UI_WIDGET_PANEL_H

#include "../../display/animation.h"
#include "../../display/theme.h"
#include "widget_base.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_widget_base_t base;
  char title[128];
  bool draggable;
  bool closable;
  bool is_open;
  civ_panel_anim_t anim;
  uint32_t color_bg;
  uint32_t color_border;
  int border_width;
  int corner_radius;
  /* Drag state */
  bool dragging;
  float drag_offset_x;
  float drag_offset_y;
} civ_panel_t;

civ_panel_t *civ_panel_create(const char *id, float x, float y, float w, float h,
                              const char *title);
void civ_panel_destroy(civ_panel_t *panel);

void civ_panel_open(civ_panel_t *panel);
void civ_panel_close(civ_panel_t *panel);
void civ_panel_toggle(civ_panel_t *panel);

void civ_panel_update(civ_panel_t *panel, civ_input_state_t *input, float dt);
void civ_panel_render(civ_panel_t *panel, SDL_Renderer *r);

/* Render panel chrome (background, border, title bar). Content rendered by caller. */
void civ_panel_render_chrome(civ_panel_t *panel, SDL_Renderer *r, int win_w,
                             int win_h);

#ifdef __cplusplus
}
#endif
#endif
