/**
 * @file scene_main_menu.c
 * @brief Main menu — widget-based with save picker popup
 */
#include "core/game.h"
#include "core/profile.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include "ui/widget/button.h"
#include "ui/window_mgr.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

extern civ_window_mgr_t *g_window_mgr;

static civ_font_t              *font_title = NULL;
static civ_font_t              *font_button = NULL;
static civ_widget_button_t     *btn_continue = NULL;
static civ_widget_button_t     *btn_new_game = NULL;
static civ_widget_button_t     *btn_load_game = NULL;
static civ_widget_button_t     *btn_logout = NULL;
static civ_widget_button_t     *btn_exit = NULL;

/* Save picker popup state */
static int   save_picker_id = -1;
static char  save_slots[16][64];
static int   save_slot_count = 0;
static char  *save_selected = NULL;

/* ── Save picker popup render callback ─────────────────────────── */
static void save_picker_render(SDL_Renderer *r, int wx, int wy,
                                int ww, int wh, void *ud, civ_input_state_t *in) {
  (void)ud; (void)in;

  int dy = wy + 30;
  if (!font_button) return;

  civ_font_render_aligned(r, font_button, "SELECT A SAVE TO LOAD",
      wx + 12, dy, ww - 24, 22, g_theme.warning,
      CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 28;

  if (save_slot_count == 0) {
    civ_font_render_aligned(r, font_button, "No save files found.",
        wx + 12, dy, ww - 24, 20, g_theme.text_dim,
        CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    dy += 24;
  }

  for (int i = 0; i < save_slot_count; i++) {
    civ_render_rect_filled_alpha(r, wx + 8, dy, ww - 16, 28,
        g_theme.bg_dark, 180);
    civ_render_rect_outline(r, wx + 8, dy, ww - 16, 28,
        g_theme.hud_border, 1);
    civ_font_render_aligned(r, font_button, save_slots[i],
        wx + 16, dy, ww - 32, 28, g_theme.text_secondary,
        CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    dy += 32;
  }
}

/* ── Save picker input callback ────────────────────────────────── */
static bool save_picker_input(int wx, int wy, int ww, int wh,
                               void *ud, civ_input_state_t *in) {
  (void)ud; (void)wh; (void)wy;
  if (!in->mouse_left_pressed) return false;

  int dy = wy + 58;
  for (int i = 0; i < save_slot_count; i++) {
    if (in->mouse_x >= wx + 8 && in->mouse_x <= wx + ww - 8 &&
        in->mouse_y >= dy && in->mouse_y <= dy + 28) {
      save_selected = save_slots[i];
      return true;
    }
    dy += 32;
  }
  return in->mouse_x >= wx && in->mouse_x <= wx + ww &&
         in->mouse_y >= wy && in->mouse_y <= wy + wh;
}

/* ── Open save picker popup ────────────────────────────────────── */
static void open_save_picker(const char *profile_id) {
  if (!g_window_mgr || !profile_id) return;

  /* Refresh save list */
  char **slots = NULL;
  save_slot_count = civ_profile_list_saves(profile_id, &slots);
  save_slot_count = save_slot_count > 16 ? 16 : save_slot_count;
  for (int i = 0; i < save_slot_count; i++)
    strncpy(save_slots[i], slots[i], 63);
  civ_profile_free_list(slots, save_slot_count > 16 ? 16 : save_slot_count);

  save_selected = NULL;

  int px = 200, py = 100, pw = 400, ph = save_slot_count * 32 + 100;
  if (ph < 200) ph = 200;
  save_picker_id = civ_window_mgr_add(g_window_mgr, CIV_WIN_POPUP,
      "Load Game", px, py, pw, ph,
      save_picker_render, save_picker_input, NULL);
}

static void init(void) {
  font_title  = civ_font_load_system("Segoe UI", 28);
  font_button = civ_font_load_system("Segoe UI", 16);
  save_picker_id = -1;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  float dt = 1.0f / 60.0f;

  /* Handle save picker selection */
  if (save_picker_id >= 0 && save_selected && game->current_profile) {
    char path[256];
    if (civ_profile_get_save_path(game->current_profile->id,
                                   save_selected, path, sizeof(path))) {
      civ_result_t r = civ_game_load(game, path);
      if (r.error == CIV_OK) {
        civ_window_mgr_remove(g_window_mgr, save_picker_id);
        save_picker_id = -1;
        civ_scene_manager_switch(SCENE_GAME);
        return;
      }
    }
    /* Failed — close picker */
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }

  /* Close picker on ESC */
  if (save_picker_id >= 0 && input->esc_pressed) {
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }

  /* Layout buttons */
  int bw = 260, bh = 40, bx = 40, by = 220, gap = 14;
  if (btn_continue) {
    btn_continue->base.x = (float)bx; btn_continue->base.y = (float)by;
    civ_widget_button_update(btn_continue, input, dt);
    by += bh + gap;
  }
  if (btn_new_game) {
    btn_new_game->base.x = (float)bx; btn_new_game->base.y = (float)by;
    civ_widget_button_update(btn_new_game, input, dt);
    by += bh + gap;
  }
  if (btn_load_game) {
    btn_load_game->base.x = (float)bx; btn_load_game->base.y = (float)by;
    civ_widget_button_update(btn_load_game, input, dt);
    by += bh + gap;
  }
  if (btn_logout) {
    btn_logout->base.x = (float)bx; btn_logout->base.y = (float)by;
    civ_widget_button_update(btn_logout, input, dt);
    by += bh + gap;
  }
  if (btn_exit) {
    btn_exit->base.x = (float)bx; btn_exit->base.y = (float)by;
    civ_widget_button_update(btn_exit, input, dt);
  }

  /* Check button clicks */
  if (civ_widget_button_was_clicked(btn_continue))
    civ_scene_manager_switch(SCENE_GAME);

  if (civ_widget_button_was_clicked(btn_new_game))
    civ_scene_manager_switch(SCENE_IDENTITY);

  if (civ_widget_button_was_clicked(btn_load_game) && game->current_profile)
    open_save_picker(game->current_profile->id);

  if (civ_widget_button_was_clicked(btn_logout))
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);

  if (civ_widget_button_was_clicked(btn_exit)) {
    SDL_Event e; e.type = SDL_EVENT_QUIT; SDL_PushEvent(&e);
  }

  /* Continue button disabled if no saves */
  if (btn_continue && game->current_profile) {
    char **slots = NULL;
    int n = civ_profile_list_saves(game->current_profile->id, &slots);
    civ_widget_button_set_enabled(btn_continue, n > 0);
    civ_profile_free_list(slots, n);
  }

  if (input->esc_pressed)
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)input;
  /* Background */
  civ_render_rect_filled(r, 0, 0, win_w, win_h, g_theme.bg_deep);

  /* Glow effect bottom-right */
  civ_render_rect_filled_alpha(r, win_w - 300, win_h - 200, 300, 200,
      g_theme.primary_dark, 15);

  /* Left sidebar panel */
  int sb_w = 340;
  civ_render_rect_filled_alpha(r, 0, 0, sb_w, win_h, g_theme.bg_dark, 240);
  civ_render_line(r, sb_w, 0, sb_w, win_h, g_theme.hud_border);

  /* Title */
  if (font_title)
    civ_font_render_aligned(r, font_title, "DOMINION", 20, 40, sb_w - 40, 36,
        CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Profile badge */
  if (font_button && game->current_profile) {
    char buf[64];
    snprintf(buf, sizeof(buf), "IDENTIFIED: %s", game->current_profile->name);
    civ_render_rect_filled_alpha(r, 20, win_h - 60, sb_w - 40, 34,
        g_theme.bg_medium, 180);
    civ_font_render_aligned(r, font_button, buf, 30, win_h - 56, sb_w - 60, 30,
        g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  /* Buttons */
  if (btn_continue)   civ_widget_button_render(btn_continue, r, font_button);
  if (btn_new_game)   civ_widget_button_render(btn_new_game, r, font_button);
  if (btn_load_game)  civ_widget_button_render(btn_load_game, r, font_button);
  if (btn_logout)     civ_widget_button_render(btn_logout, r, font_button);
  if (btn_exit)       civ_widget_button_render(btn_exit, r, font_button);

  if (!btn_continue && font_button) {
    /* Lazy-create buttons (first render, after fonts loaded) */
    btn_continue  = civ_widget_button_create("cont",  0, 0, 260, 40, "CONTINUE");
    btn_new_game  = civ_widget_button_create("new",   0, 0, 260, 40, "NEW GAME");
    btn_load_game = civ_widget_button_create("load",  0, 0, 260, 40, "LOAD GAME");
    btn_logout    = civ_widget_button_create("logout",0, 0, 260, 40, "SWITCH PROFILE");
    btn_exit      = civ_widget_button_create("exit",  0, 0, 260, 40, "EXIT TO DESKTOP");
  }
}

static void destroy(void) {
  if (save_picker_id >= 0 && g_window_mgr) {
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }
  if (btn_continue)  civ_widget_button_destroy(btn_continue),  btn_continue = NULL;
  if (btn_new_game)  civ_widget_button_destroy(btn_new_game),  btn_new_game = NULL;
  if (btn_load_game) civ_widget_button_destroy(btn_load_game), btn_load_game = NULL;
  if (btn_logout)    civ_widget_button_destroy(btn_logout),    btn_logout = NULL;
  if (btn_exit)      civ_widget_button_destroy(btn_exit),      btn_exit = NULL;
  if (font_title)    civ_font_destroy(font_title), font_title = NULL;
  if (font_button)   civ_font_destroy(font_button), font_button = NULL;
}

civ_scene_t scene_main_menu = {
  .init = init, .update = update, .render = render,
  .destroy = destroy, .next_scene_id = -1
};
