/**
 * @file scene_main_menu.c
 * @brief Main menu — fully Nuklear-rendered
 */
#include "core/game.h"
#include "core/profile.h"
#include "display/theme.h"
#include "engine/font.h"
#include "ui/confirm_dialog.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include "ui/window_mgr.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

extern civ_window_mgr_t *g_window_mgr;

static int   save_picker_id = -1;
static char  save_slots[16][64];
static int   save_slot_count = 0;
static char  *save_selected = NULL;

/* ── Save picker callbacks ───────────────────────────────────── */
static void save_picker_render(SDL_Renderer *r, int wx, int wy,
                                int ww, int wh, void *ud, civ_input_state_t *in) {
  (void)ud; (void)in;
  if (!g_nk_ctx) return;
  struct nk_context *nk = g_nk_ctx;
  if (nk_begin(nk, "Load Game", nk_rect((float)wx, (float)wy, (float)ww, (float)wh),
               NK_WINDOW_TITLE|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
    nk_layout_row_dynamic(nk, 22, 1);
    if (save_slot_count == 0)
      nk_label(nk, "No save files found.", NK_TEXT_LEFT);
    for (int i = 0; i < save_slot_count; i++) {
      nk_layout_row_dynamic(nk, 28, 1);
      if (nk_button_label(nk, save_slots[i]))
        save_selected = save_slots[i];
    }
  }
  nk_end(nk);
}

static bool save_picker_input(int wx, int wy, int ww, int wh,
                               void *ud, civ_input_state_t *in) {
  (void)ud; (void)wx; (void)wy; (void)ww; (void)wh;
  return in->mouse_left_pressed;
}

static void open_save_picker(const char *profile_id) {
  if (!g_window_mgr || !profile_id) return;
  char **slots = NULL;
  save_slot_count = civ_profile_list_saves(profile_id, &slots);
  save_slot_count = save_slot_count > 16 ? 16 : save_slot_count;
  for (int i = 0; i < save_slot_count; i++)
    strncpy(save_slots[i], slots[i], 63);
  civ_profile_free_list(slots, save_slot_count > 16 ? 16 : save_slot_count);
  save_selected = NULL;
  save_picker_id = civ_window_mgr_add(g_window_mgr, CIV_WIN_POPUP,
      "Load Game", 200, 100, 400, save_slot_count * 32 + 100,
      save_picker_render, save_picker_input, NULL);
}

/* ── Scene lifecycle ─────────────────────────────────────────── */
static void init(void) {
  save_picker_id = -1;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  /* Handle save picker selection */
  if (save_picker_id >= 0 && save_selected && game->current_profile) {
    char path[256];
    if (civ_profile_get_save_path(game->current_profile->id,
                                   save_selected, path, sizeof(path))) {
      if (civ_game_load(game, path).error == CIV_OK) {
        civ_window_mgr_remove(g_window_mgr, save_picker_id);
        save_picker_id = -1;
        civ_scene_manager_switch(SCENE_GAME);
        return;
      }
    }
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }
  if (save_picker_id >= 0 && input->esc_pressed) {
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }

  /* Check confirmation dialog */
  int cr = civ_confirm_result();
  if (cr == 1) { SDL_Event e; e.type = SDL_EVENT_QUIT; SDL_PushEvent(&e); }
  if (cr == -1) { /* cancelled, do nothing */ }
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)r; (void)input;
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  /* Background */
  if (nk_begin(nk, "Dominion", nk_rect(0, 0, (float)win_w, (float)win_h),
               NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_space_begin(nk, NK_STATIC, win_h, 1);

    /* Title area */
    nk_layout_space_push(nk, nk_rect(40, 50, 300, 44));
    nk_label(nk, "DOMINION", NK_TEXT_LEFT);

    /* Menu buttons */
    float by = 220;
    const char *labels[] = {"CONTINUE", "NEW GAME", "LOAD GAME",
                            "SWITCH PROFILE", "EXIT TO DESKTOP"};
    for (int i = 0; i < 5; i++) {
      nk_layout_space_push(nk, nk_rect(40, by, 260, 38));
      if (nk_button_label(nk, labels[i])) {
        switch (i) {
        case 0: civ_scene_manager_switch(SCENE_GAME); break;
        case 1: civ_scene_manager_switch(SCENE_IDENTITY); break;
        case 2: if (game->current_profile) open_save_picker(game->current_profile->id); break;
        case 3: civ_scene_manager_switch(SCENE_PROFILE_SELECT); break;
        case 4: civ_confirm_show("Exit Dominion", "Are you sure you want to exit?"); break;
        }
      }
      by += 48;
    }

    /* Profile badge at bottom */
    if (game->current_profile) {
      char buf[64];
      snprintf(buf, sizeof(buf), "Signed in as: %s", game->current_profile->name);
      nk_layout_space_push(nk, nk_rect(40, (float)win_h - 60, 300, 24));
      nk_label(nk, buf, NK_TEXT_LEFT);
    }

    nk_layout_space_end(nk);
  }
  nk_end(nk);
}

static void destroy(void) {
  if (save_picker_id >= 0 && g_window_mgr) {
    civ_window_mgr_remove(g_window_mgr, save_picker_id);
    save_picker_id = -1;
  }
}

civ_scene_t scene_main_menu = {
  .init = init, .update = update, .render = render,
  .destroy = destroy, .next_scene_id = -1
};
