/**
 * @file window_mgr.h
 * @brief UI window manager — full screens, overlays, sidebars, popups, HUD
 *
 * Manages a z-ordered stack of windows. Each window has a type, rect,
 * title bar (optional), and render callback. Input is routed to the
 * topmost window first.
 */
#ifndef CIV_UI_WINDOW_MGR_H
#define CIV_UI_WINDOW_MGR_H

#include "../engine/font.h"
#include "../engine/input.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_WINDOW_MAX 16

typedef enum {
  CIV_WIN_FULLSCREEN,  /* takes entire display */
  CIV_WIN_OVERLAY,     /* centered modal panel */
  CIV_WIN_SIDEBAR,     /* docked left/right */
  CIV_WIN_POPUP,       /* small floating window */
  CIV_WIN_HUD,         /* always-visible chrome element */
} civ_window_type_t;

typedef void (*civ_window_render_fn)(SDL_Renderer *r, int wx, int wy,
                                     int ww, int wh, void *userdata,
                                     civ_input_state_t *input);
typedef bool (*civ_window_input_fn)(int wx, int wy, int ww, int wh,
                                    void *userdata, civ_input_state_t *input);

typedef struct {
  char                title[48];
  civ_window_type_t   type;
  int                 x, y, w, h;
  int                 z_index;
  bool                visible;
  bool                has_title_bar;
  bool                draggable;
  bool                closable;
  bool                dragging;
  int                 drag_ox, drag_oy;
  uint32_t            color_border;
  uint32_t            color_title;
  civ_window_render_fn render;
  civ_window_input_fn  handle_input;
  void               *userdata;
} civ_ui_window_t;

typedef struct {
  civ_ui_window_t windows[CIV_WINDOW_MAX];
  int             count;
  int             next_z;
  bool            needs_redraw;
} civ_window_mgr_t;

void civ_window_mgr_init(civ_window_mgr_t *mgr);
int  civ_window_mgr_add(civ_window_mgr_t *mgr, civ_window_type_t type,
                        const char *title, int x, int y, int w, int h,
                        civ_window_render_fn render,
                        civ_window_input_fn input_fn, void *userdata);
void civ_window_mgr_remove(civ_window_mgr_t *mgr, int idx);
void civ_window_mgr_show(civ_window_mgr_t *mgr, int idx, bool show);

/* Process input for all visible windows (top-down). Returns true if handled. */
bool civ_window_mgr_input(civ_window_mgr_t *mgr, civ_input_state_t *input);

/* Render all visible windows (bottom-up). */
void civ_window_mgr_render(civ_window_mgr_t *mgr, SDL_Renderer *r,
                           civ_font_t *font);

/* Render chrome for one window (background, border, title bar) */
void civ_window_render_chrome(SDL_Renderer *r, civ_ui_window_t *win,
                              civ_font_t *font);

#ifdef __cplusplus
}
#endif
#endif
