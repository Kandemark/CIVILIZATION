/**
 * @file scene_profile_select.c
 * @brief Scene for selecting a player profile
 */

#include "../../../include/core/game.h"
#include "../../../include/core/profile.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/button.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local state */
static civ_font_t *font_title = NULL;
static civ_font_t *font_button = NULL;

static char **profile_list = NULL;
static int profile_count = 0;

static civ_button_t *profile_buttons = NULL;
static civ_button_t btn_new_profile;
static bool has_profiles = false;

static void refresh_profile_list(void) {
  if (profile_list) {
    civ_profile_free_list(profile_list, profile_count);
    profile_list = NULL;
  }
  if (profile_buttons) {
    free(profile_buttons);
    profile_buttons = NULL;
  }

  profile_count = civ_profile_list(&profile_list);
  has_profiles = (profile_count > 0);

  if (has_profiles) {
    profile_buttons =
        (civ_button_t *)malloc(profile_count * sizeof(civ_button_t));
    for (int i = 0; i < profile_count; i++) {
      profile_buttons[i] =
          civ_button_create(100 + i, 0, 0, 400, 50, profile_list[i]);
    }
  }
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 48);
  font_button = civ_font_load_system("Segoe UI", 24);

  btn_new_profile = civ_button_create(10, 0, 0, 400, 50, "CREATE NEW PROFILE");
  btn_new_profile.color = CIV_COLOR_ACCENT;

  refresh_profile_list();
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  /* Interaction handled in render/civ_button_render as per existing UI pattern
   */
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  /* 1. Background atmospheric */
  civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(renderer, win_w / 2 - 300, 0, 600, win_h,
                               0x070B1A, 100);

  /* 2. Title Section */
  if (font_title) {
    civ_font_render_aligned(renderer, font_title, "IDENTIFY OPERATOR", 0, 80,
                            win_w, 60, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);

    civ_font_render_aligned(renderer, font_button,
                            "SELECT AN EXISTING PROFILE OR INITIALIZE NEW", 0,
                            140, win_w, 30, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* 3. Profile Cards List */
  int card_w = 400;
  int card_h = 60;
  int start_x = (win_w - card_w) / 2;
  int start_y = 220;
  int gap = 12;

  for (int i = 0; i < profile_count; i++) {
    profile_buttons[i].x = start_x;
    profile_buttons[i].y = start_y + (i * (card_h + gap));
    profile_buttons[i].w = card_w;
    profile_buttons[i].h = card_h;

    /* Highlight existing profiles differently if active? (Future) */

    if (civ_button_render(renderer, font_button, &profile_buttons[i], input)) {
      civ_player_profile_t *profile = civ_profile_load(profile_list[i]);
      if (profile) {
        if (game->current_profile) {
          civ_profile_destroy(game->current_profile);
        }
        game->current_profile = profile;
        printf("Operator identified: %s. Transitioning to Command Center.\n",
               profile->name);
        civ_scene_manager_switch(SCENE_MAIN_MENU);
      }
    }
  }

  /* 4. New Profile Button (Floating at bottom) */
  int new_btn_w = 400;
  btn_new_profile.x = (win_w - new_btn_w) / 2;
  btn_new_profile.y = win_h - 120;
  btn_new_profile.w = new_btn_w;
  btn_new_profile.h = 50;

  if (civ_button_render(renderer, font_button, &btn_new_profile, input)) {
    civ_scene_manager_switch(SCENE_PROFILE_CREATE);
  }
}

static void destroy(void) {
  if (font_title)
    civ_font_destroy(font_title);
  if (font_button)
    civ_font_destroy(font_button);

  if (profile_list) {
    civ_profile_free_list(profile_list, profile_count);
    profile_list = NULL;
  }
  if (profile_buttons) {
    free(profile_buttons);
    profile_buttons = NULL;
  }
  font_title = NULL;
  font_button = NULL;
}

civ_scene_t scene_profile_select = {.init = init,
                                    .update = update,
                                    .render = render,
                                    .destroy = destroy,
                                    .next_scene_id = -1};
