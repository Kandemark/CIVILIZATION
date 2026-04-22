/**
 * @file window_mgr.c
 * @brief UI window manager implementation
 */
#include "ui/window_mgr.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

void civ_window_mgr_init(civ_window_mgr_t *mgr) {
  memset(mgr, 0, sizeof(*mgr));
}

int civ_window_mgr_add(civ_window_mgr_t *mgr, civ_window_type_t type,
                       const char *title, int x, int y, int w, int h,
                       civ_window_render_fn render,
                       civ_window_input_fn input_fn, void *userdata) {
  if (!mgr || mgr->count >= CIV_WINDOW_MAX) return -1;

  int idx = mgr->count++;
  civ_ui_window_t *win = &mgr->windows[idx];
  memset(win, 0, sizeof(*win));

  win->type = type;
  win->x = x; win->y = y; win->w = w; win->h = h;
  win->z_index = mgr->next_z++;
  win->visible = true;
  win->render = render;
  win->handle_input = input_fn;
  win->userdata = userdata;

  if (title) strncpy(win->title, title, sizeof(win->title) - 1);

  /* Defaults by type */
  switch (type) {
  case CIV_WIN_OVERLAY:
    win->has_title_bar = true; win->draggable = true; win->closable = true;
    win->color_border = g_theme.primary; win->color_title = g_theme.bg_medium;
    break;
  case CIV_WIN_SIDEBAR:
    win->has_title_bar = false; win->draggable = false; win->closable = false;
    win->color_border = g_theme.primary;
    break;
  case CIV_WIN_POPUP:
    win->has_title_bar = true; win->draggable = true; win->closable = true;
    win->color_border = 0x00FFD2; win->color_title = g_theme.bg_dark;
    break;
  case CIV_WIN_HUD:
    win->has_title_bar = false; win->draggable = false; win->closable = false;
    win->color_border = 0;
    break;
  case CIV_WIN_FULLSCREEN:
  default:
    win->has_title_bar = false; win->draggable = false; win->closable = false;
    break;
  }

  return idx;
}

void civ_window_mgr_remove(civ_window_mgr_t *mgr, int idx) {
  if (!mgr || idx < 0 || idx >= mgr->count) return;
  /* Shift remaining windows down */
  for (int i = idx; i < mgr->count - 1; i++)
    mgr->windows[i] = mgr->windows[i + 1];
  mgr->count--;
}

void civ_window_mgr_show(civ_window_mgr_t *mgr, int idx, bool show) {
  if (!mgr || idx < 0 || idx >= mgr->count) return;
  mgr->windows[idx].visible = show;
}

/* ── Chrome rendering ───────────────────────────────────────────────── */
void civ_window_render_chrome(SDL_Renderer *r, civ_ui_window_t *win,
                              civ_font_t *font) {
  if (!r || !win) return;
  int x = win->x, y = win->y, w = win->w, h = win->h;

  /* Background */
  civ_render_rect_filled_alpha(r, x, y, w, h, g_theme.bg_dark, 235);

  /* Border */
  if (win->color_border)
    civ_render_rect_outline(r, x, y, w, h, win->color_border, 1);

  /* Title bar */
  if (win->has_title_bar) {
    int th = 26;
    civ_render_rect_filled_alpha(r, x, y, w, th, win->color_title, 240);
    civ_render_line(r, x, y + th, x + w, y + th,
                    win->color_border ? win->color_border : 0x1A2A3A);
    if (font && win->title[0])
      civ_font_render_aligned(r, font, win->title, x + 8, y, w - 30, th,
                              0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    /* Close button */
    if (win->closable) {
      int cx = x + w - 22, cy = y + 3, cs = 20;
      civ_font_render_aligned(r, font, "x", cx, cy, cs, cs, 0x888888,
                              CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    }
  }
}

/* ── Input ──────────────────────────────────────────────────────────── */
bool civ_window_mgr_input(civ_window_mgr_t *mgr, civ_input_state_t *input) {
  if (!mgr || !input) return false;

  /* Process top-down (highest z first) */
  for (int i = mgr->count - 1; i >= 0; i--) {
    civ_ui_window_t *win = &mgr->windows[i];
    if (!win->visible) continue;

    /* Check if click is in this window */
    bool in_win = civ_input_is_mouse_over(input, win->x, win->y, win->w, win->h);

    /* Close button click */
    if (win->closable && in_win && input->mouse_left_pressed) {
      int th = win->has_title_bar ? 26 : 0;
      int cx = win->x + win->w - 22, cy = win->y + 3;
      if (input->mouse_x >= cx && input->mouse_x <= cx + 20 &&
          input->mouse_y >= cy && input->mouse_y <= cy + 20) {
        win->visible = false;
        return true;
      }
    }

    /* Drag title bar */
    if (win->draggable && win->has_title_bar && in_win && input->mouse_left_pressed) {
      int th = 26;
      if (input->mouse_y < win->y + th && input->mouse_x < win->x + win->w - 24) {
        win->dragging = true;
        win->drag_ox = input->mouse_x - win->x;
        win->drag_oy = input->mouse_y - win->y;
        /* Bring to top */
        win->z_index = mgr->next_z++;
        return true;
      }
    }
    if (win->dragging && input->mouse_left_down) {
      win->x = input->mouse_x - win->drag_ox;
      win->y = input->mouse_y - win->drag_oy;
      return true;
    }
    if (win->dragging && !input->mouse_left_down) {
      win->dragging = false;
    }

    /* Delegate to window's input handler if click is inside */
    if (in_win && win->handle_input) {
      if (win->handle_input(win->x, win->y, win->w, win->h, win->userdata, input))
        return true;
    }

    /* If click is in this window, stop propagating (for overlays) */
    if (in_win && (win->type == CIV_WIN_OVERLAY || win->type == CIV_WIN_POPUP)) {
      if (input->mouse_left_pressed) return true;
    }
  }
  return false;
}

/* ── Render ──────────────────────────────────────────────────────────── */
void civ_window_mgr_render(civ_window_mgr_t *mgr, SDL_Renderer *r,
                           civ_font_t *font) {
  if (!mgr || !r) return;

  /* Render bottom-up (lowest z first) */
  for (int i = 0; i < mgr->count; i++) {
    civ_ui_window_t *win = &mgr->windows[i];
    if (!win->visible) continue;

    /* Render chrome for non-fullscreen windows */
    if (win->type != CIV_WIN_FULLSCREEN && win->type != CIV_WIN_HUD)
      civ_window_render_chrome(r, win, font);

    /* Call the window's render callback */
    if (win->render)
      win->render(r, win->x, win->y, win->w, win->h, win->userdata, NULL);
  }
}
