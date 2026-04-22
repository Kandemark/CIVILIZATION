#include "core/game.h"
#include "core/profile.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <string.h>

static char name_buf[32] = "";
static int  name_len = 0;

static void init(void) {
  name_buf[0] = '\0'; name_len = 0;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (input->esc_pressed) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT); return;
  }
  /* Text input */
  if (input->text_input[0] && name_len < 30) {
    strcat(name_buf, input->text_input); name_len = (int)strlen(name_buf);
  }
  if (input->backspace_pressed && name_len > 0) {
    name_buf[--name_len] = '\0';
  }
  /* Enter to create */
  if (input->enter_pressed && name_len > 0) {
    civ_player_profile_t *p = civ_profile_create(name_buf);
    if (p) { civ_profile_save(p); game->current_profile = p; civ_scene_manager_switch(SCENE_MAIN_MENU); }
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)r; (void)game; (void)input;
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  float pw = 400, ph = 260;
  float px = ((float)win_w - pw) / 2, py = ((float)win_h - ph) / 2;

  if (nk_begin(nk, "Create Profile", nk_rect(px, py, pw, ph),
               NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
    nk_layout_row_dynamic(nk, 28, 1);
    nk_label(nk, "CREATE PROFILE", NK_TEXT_CENTERED);
    nk_layout_row_dynamic(nk, 12, 1);

    nk_layout_row_dynamic(nk, 20, 1);
    nk_label(nk, "Enter your name:", NK_TEXT_LEFT);

    nk_layout_row_dynamic(nk, 36, 1);
    nk_flags active = nk_edit_string_zero_terminated(nk, NK_EDIT_SIMPLE,
        name_buf, sizeof(name_buf), nk_filter_default);

    nk_layout_row_dynamic(nk, 16, 1);

    /* Buttons */
    nk_layout_row_dynamic(nk, 38, 2);
    if (nk_button_label(nk, "CREATE")) {
      if (name_len > 0) {
        civ_player_profile_t *p = civ_profile_create(name_buf);
        if (p) { civ_profile_save(p); game->current_profile = p; civ_scene_manager_switch(SCENE_MAIN_MENU); }
      }
    }
    if (nk_button_label(nk, "CANCEL"))
      civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
  nk_end(nk);
}

static void destroy(void) {}

civ_scene_t scene_profile_create = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
