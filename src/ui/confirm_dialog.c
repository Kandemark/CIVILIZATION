/**
 * @file confirm_dialog.c
 * @brief Confirmation dialog implementation
 */
#include "ui/confirm_dialog.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/window_mgr.h"
#include <stdio.h>
#include <string.h>

extern civ_window_mgr_t *g_window_mgr;

static int  dialog_id = -1;
static int  dialog_result = 0;
static char dialog_title[48];
static char dialog_message[256];

static void confirm_render(SDL_Renderer *r, int wx, int wy,
                            int ww, int wh, void *ud, civ_input_state_t *in) {
  (void)ud; (void)in;
  /* Content area below title bar */
  int cy = wy + 30;
  civ_font_t *font = civ_font_load_system("Segoe UI", 14);
  if (!font) return;

  if (dialog_message[0])
    civ_font_render_aligned(r, font, dialog_message,
        wx + 16, cy, ww - 32, wh - 70,
        g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  /* Confirm button */
  int by = wy + wh - 44;
  civ_render_rect_filled_alpha(r, wx + 20, by, 120, 30,
      g_theme.primary_dark, 200);
  civ_render_rect_outline(r, wx + 20, by, 120, 30, g_theme.primary, 1);
  civ_font_render_aligned(r, font, "CONFIRM",
      wx + 20, by, 120, 30, g_theme.text_primary,
      CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* Cancel button */
  civ_render_rect_filled_alpha(r, wx + ww - 140, by, 120, 30,
      g_theme.bg_medium, 200);
  civ_render_rect_outline(r, wx + ww - 140, by, 120, 30,
      g_theme.hud_border, 1);
  civ_font_render_aligned(r, font, "CANCEL",
      wx + ww - 140, by, 120, 30, g_theme.text_secondary,
      CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  civ_font_destroy(font);
}

static bool confirm_input(int wx, int wy, int ww, int wh,
                           void *ud, civ_input_state_t *in) {
  (void)ud; (void)wh;
  if (!in->mouse_left_pressed) return false;

  int by = wy + wh - 44;
  /* Confirm button area */
  if (in->mouse_x >= wx + 20 && in->mouse_x <= wx + 140 &&
      in->mouse_y >= by && in->mouse_y <= by + 30) {
    dialog_result = 1;
    return true;
  }
  /* Cancel button area */
  if (in->mouse_x >= wx + ww - 140 && in->mouse_x <= wx + ww - 20 &&
      in->mouse_y >= by && in->mouse_y <= by + 30) {
    dialog_result = -1;
    return true;
  }
  return in->mouse_x >= wx && in->mouse_x <= wx + ww &&
         in->mouse_y >= wy && in->mouse_y <= wy + wh;
}

void civ_confirm_show(const char *title, const char *message) {
  if (!g_window_mgr) return;

  if (title) strncpy(dialog_title, title, sizeof(dialog_title) - 1);
  if (message) strncpy(dialog_message, message, sizeof(dialog_message) - 1);
  dialog_result = 0;

  dialog_id = civ_window_mgr_add(g_window_mgr, CIV_WIN_OVERLAY,
      dialog_title, 0, 0, 420, 180,
      confirm_render, confirm_input, NULL);

  /* Center the dialog */
  if (dialog_id >= 0) {
    /* Approximate centering — caller should set actual position.
       We rely on window_mgr's drag to let the user position it. */
    g_window_mgr->windows[dialog_id].x = 200;
    g_window_mgr->windows[dialog_id].y = 180;
  }
}

int civ_confirm_result(void) {
  /* If dialog was closed via the X button, treat as cancel */
  if (dialog_id >= 0 && g_window_mgr &&
      !g_window_mgr->windows[dialog_id].visible) {
    if (dialog_result == 0) dialog_result = -1;
    dialog_id = -1;
  }
  return dialog_result;
}

bool civ_confirm_is_open(void) {
  return dialog_id >= 0 && g_window_mgr &&
         g_window_mgr->windows[dialog_id].visible;
}

void civ_confirm_close(void) {
  if (dialog_id >= 0 && g_window_mgr) {
    civ_window_mgr_remove(g_window_mgr, dialog_id);
    dialog_id = -1;
    if (dialog_result == 0) dialog_result = -1;
  }
}
