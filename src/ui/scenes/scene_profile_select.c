#include "../../../include/core/game.h"
#include "../../../include/core/profile.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include "../../../include/ui/widget/button.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

static civ_font_t             *font_title = NULL;
static civ_font_t             *font_button = NULL;
static char                  **profile_list = NULL;
static int                     profile_count = 0;
static civ_widget_button_t   **profile_buttons = NULL;
static civ_widget_button_t    *btn_new_profile = NULL;

static void refresh_profile_list(void) {
  if (profile_list) civ_profile_free_list(profile_list, profile_count);
  profile_list = NULL;
  if (profile_buttons) {
    for (int i = 0; i < profile_count; i++)
      civ_widget_button_destroy(profile_buttons[i]);
    free(profile_buttons);
    profile_buttons = NULL;
  }
  profile_count = civ_profile_list(&profile_list);
  if (profile_count > 0) {
    profile_buttons = (civ_widget_button_t **)malloc(
        (size_t)profile_count * sizeof(civ_widget_button_t *));
    for (int i = 0; i < profile_count; i++) {
      char id[32];
      snprintf(id, sizeof(id), "prof_%d", i);
      profile_buttons[i] = civ_widget_button_create(id, 0, 0, 400, 50,
                                                    profile_list[i]);
    }
  }
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 32);
  font_button = civ_font_load_system("Segoe UI", 18);
  btn_new_profile = civ_widget_button_create("new_profile", 0, 0, 400, 50,
                                             "CREATE NEW PROFILE");
  btn_new_profile->color_bg = 0x003A2A;
  btn_new_profile->color_hover = 0x005544;
  refresh_profile_list();
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  (void)game;
  float dt = 1.0f / 60.0f;

  for (int i = 0; i < profile_count; i++)
    civ_widget_button_update(profile_buttons[i], input, dt);
  civ_widget_button_update(btn_new_profile, input, dt);

  for (int i = 0; i < profile_count; i++) {
    if (civ_widget_button_was_clicked(profile_buttons[i])) {
      civ_player_profile_t *p = civ_profile_load(profile_list[i]);
      if (p) {
        if (game->current_profile) civ_profile_destroy(game->current_profile);
        game->current_profile = p;
        civ_scene_manager_switch(SCENE_MAIN_MENU);
      }
    }
  }
  if (civ_widget_button_was_clicked(btn_new_profile))
    civ_scene_manager_switch(SCENE_PROFILE_CREATE);
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  (void)input;
  civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(r, win_w / 2 - 300, 0, 600, win_h, 0x070B1A, 100);

  if (font_title) {
    civ_font_render_aligned(r, font_title, "IDENTIFY OPERATOR", 0, 80, win_w, 60,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    civ_font_render_aligned(r, font_button, "SELECT A PROFILE OR CREATE NEW", 0,
                            140, win_w, 30, CIV_COLOR_TEXT_DIM, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  }

  int card_w = 400, card_h = 60, start_x = (win_w - card_w) / 2, start_y = 220,
      gap = 12;
  for (int i = 0; i < profile_count; i++) {
    civ_widget_button_set_position(profile_buttons[i], (float)start_x,
                                   (float)(start_y + i * (card_h + gap)));
    profile_buttons[i]->base.w = (float)card_w;
    profile_buttons[i]->base.h = (float)card_h;
    civ_widget_button_render(profile_buttons[i], r, font_button);
  }

  int nw = 400;
  btn_new_profile->base.x = (float)((win_w - nw) / 2);
  btn_new_profile->base.y = (float)(win_h - 120);
  btn_new_profile->base.w = (float)nw;
  btn_new_profile->base.h = 50;
  civ_widget_button_render(btn_new_profile, r, font_button);
  (void)game;
}

static void destroy(void) {
  if (font_title)   civ_font_destroy(font_title),   font_title = NULL;
  if (font_button)  civ_font_destroy(font_button),  font_button = NULL;
  if (profile_list) civ_profile_free_list(profile_list, profile_count);
  if (profile_buttons) {
    for (int i = 0; i < profile_count; i++)
      civ_widget_button_destroy(profile_buttons[i]);
    free(profile_buttons);
  }
  civ_widget_button_destroy(btn_new_profile);
}

civ_scene_t scene_profile_select = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
