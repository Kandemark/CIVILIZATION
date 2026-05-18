/**
 * @file drawer.h
 * @brief Collapsible slide-out drawer widget
 *
 * Anchors to left or right window edge. Collapses to a thin icon tab.
 * Opens with a smooth slide animation. Supports child widgets, resizing,
 * and pin/unpin (reserve content area vs overlay).
 */
#ifndef CIV_UI_WIDGET_DRAWER_H
#define CIV_UI_WIDGET_DRAWER_H

#include "../../display/animation.h"
#include "../../display/theme.h"
#include "../../engine/input.h"
#include "../icon/icon_atlas.h"
#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct civ_drawer {
  civ_widget_base_t base;

  /* Edge anchoring */
  bool right_side;       /* true = right edge, false = left edge */

  /* State */
  bool is_open;
  bool is_pinned;        /* when pinned, content area is reserved (push mode) */
  bool is_resizing;
  bool hover_tab;

  /* Dimensions */
  float collapsed_w;     /* width when closed (just the tab) */
  float open_w;          /* width when fully open */
  float current_w;       /* animated current width */
  float min_w, max_w;    /* resize bounds */

  /* Animation */
  civ_tween_t width_tween;
  civ_tween_t alpha_tween;

  /* Tab (visible when collapsed or as a pull handle) */
  civ_icon_id_t tab_icon;
  char          tab_label[32];
  int           tab_width;   /* width of the pull-tab strip */
  uint32_t      tab_color;
  uint32_t      tab_color_hover;

  /* Resize drag state */
  float resize_grab_x;
  float resize_start_w;
  int   resize_handle_w;  /* px width of the resize zone */

  /* Visual */
  uint32_t color_bg;
  uint32_t color_border;
  int      border_width;
} civ_drawer_t;

civ_drawer_t *civ_drawer_create(const char *id, bool right_side,
                                float open_width, const char *label,
                                civ_icon_id_t icon);
void civ_drawer_destroy(civ_drawer_t *d);

void civ_drawer_open(civ_drawer_t *d);
void civ_drawer_close(civ_drawer_t *d);
void civ_drawer_toggle(civ_drawer_t *d);
bool civ_drawer_is_open(const civ_drawer_t *d);

void civ_drawer_update(civ_drawer_t *d, civ_input_state_t *input,
                       float dt, int win_w, int win_h);
void civ_drawer_render_chrome(civ_drawer_t *d, SDL_Renderer *r,
                              int win_w, int win_h);

/* The drawer's current position/size for content layout.
 * Call this to know where to render child widgets. */
void civ_drawer_get_content_rect(civ_drawer_t *d, int win_h,
                                 float *x, float *y, float *w, float *h);

#ifdef __cplusplus
}
#endif
#endif
