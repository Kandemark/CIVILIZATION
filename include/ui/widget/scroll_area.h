/**
 * @file scroll_area.h
 * @brief Scrollable content region with clipped viewport and scrollbar
 *
 * Renders content to an off-screen texture, then blits the visible
 * portion. Supports mouse wheel scrolling and scrollbar dragging.
 */
#ifndef CIV_UI_WIDGET_SCROLL_AREA_H
#define CIV_UI_WIDGET_SCROLL_AREA_H

#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct civ_scroll_area {
  civ_widget_base_t base;
  SDL_Texture *content_tex;   /* off-screen render target */
  int          content_w, content_h; /* virtual content size */
  float        scroll_x, scroll_y;   /* scroll offset */
  float        target_scroll_x, target_scroll_y;
  bool         dragging_scrollbar;
  float        drag_start_y;
  float        drag_start_scroll;
  int          scrollbar_w;
  uint32_t     color_bg;
  uint32_t     color_scrollbar;
  uint32_t     color_scrollbar_hover;
  bool         hover_scrollbar;
} civ_scroll_area_t;

civ_scroll_area_t *civ_scroll_area_create(const char *id, float x, float y,
                                          float w, float h, int content_w,
                                          int content_h);
void civ_scroll_area_destroy(civ_scroll_area_t *sa);
void civ_scroll_area_set_content_size(civ_scroll_area_t *sa, int cw, int ch);

/* Call before rendering children: sets the SDL render target to the
 * content texture with the scroll offset applied.
 * Returns the content SDL_Renderer for caller to draw into. */
SDL_Renderer *civ_scroll_area_begin(civ_scroll_area_t *sa, SDL_Renderer *main_r);
void          civ_scroll_area_end(civ_scroll_area_t *sa, SDL_Renderer *main_r);

/* Update + render as a widget (for overlay mode) */
void civ_scroll_area_update(civ_scroll_area_t *sa, civ_input_state_t *input,
                            float dt);
void civ_scroll_area_render(civ_scroll_area_t *sa, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
