#include "core/game.h"
#include "core/profile.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <string.h>

static char **profile_list = NULL;
static int    profile_count = 0;

static void refresh(void) {
  if (profile_list) civ_profile_free_list(profile_list, profile_count);
  profile_count = civ_profile_list(&profile_list);
}

static void init(void)   { refresh(); }
static void destroy(void) {
  if (profile_list) civ_profile_free_list(profile_list, profile_count);
  profile_list = NULL; profile_count = 0;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  (void)game; (void)input;
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)r; (void)input;
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  float pw = 440, ph = (float)(profile_count * 44 + 160);
  if (ph < 280) ph = 280;
  float px = ((float)win_w - pw) / 2, py = ((float)win_h - ph) / 2;

  if (nk_begin(nk, "Select Profile", nk_rect(px, py, pw, ph),
               NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
    nk_layout_row_dynamic(nk, 28, 1);
    nk_label(nk, "IDENTIFY OPERATOR", NK_TEXT_CENTERED);
    nk_layout_row_dynamic(nk, 16, 1);
    nk_label(nk, "Select a profile", NK_TEXT_CENTERED);
    for (int i = 0; i < profile_count; i++) {
      nk_layout_row_dynamic(nk, 34, 1);
      if (nk_button_label(nk, profile_list[i])) {
        civ_player_profile_t *p = civ_profile_load(profile_list[i]);
        if (p) { game->current_profile = p; civ_scene_manager_switch(SCENE_MAIN_MENU); }
      }
    }
    nk_layout_row_dynamic(nk, 10, 1);
    nk_layout_row_dynamic(nk, 38, 1);
    if (nk_button_label(nk, "+ CREATE NEW PROFILE"))
      civ_scene_manager_switch(SCENE_PROFILE_CREATE);
  }
  nk_end(nk);
}

civ_scene_t scene_profile_select = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
