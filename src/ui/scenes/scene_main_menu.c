/**
 * @file scene_main_menu.c
 * @brief Scene for the main menu after profile selection
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

/* Local state */
static civ_font_t *font_title = NULL;
static civ_font_t *font_profile = NULL;
static civ_font_t *font_button = NULL;

static civ_button_t btn_continue;
static civ_button_t btn_new_game;
static civ_button_t btn_load_game;
static civ_button_t btn_logout;
static civ_button_t btn_exit;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 64);
  font_profile = civ_font_load_system("Segoe UI", 20);
  font_button = civ_font_load_system("Segoe UI", 28);

  btn_continue = civ_button_create(30, 0, 0, 300, 50, "CONTINUE");
  btn_new_game = civ_button_create(31, 0, 0, 300, 50, "NEW GAME");
  btn_load_game = civ_button_create(32, 0, 0, 300, 50, "LOAD SLOT");
  btn_logout = civ_button_create(33, 0, 0, 300, 50, "SWITCH PROFILE");
  btn_exit = civ_button_create(34, 0, 0, 300, 50, "EXIT");

  btn_logout.color = 0x555555;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  (void)game;
  if (input->esc_pressed) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(renderer, 0, win_h - 400, 600, 400,
                               CIV_COLOR_GLOW, 40);

  int panel_w = 340;
  civ_render_rect_filled_alpha(renderer, 0, 0, panel_w, win_h,
                               CIV_COLOR_BG_MEDIUM, 180);
  civ_render_line(renderer, panel_w, 0, panel_w, win_h, 0x1A2A3A);

  if (font_title) {
    civ_font_render_aligned(renderer, font_title, "CIVILIZATION", 40, 60,
                            panel_w - 80, 60, CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);
  }

  if (font_profile && game->current_profile) {
    civ_render_rect_filled_alpha(renderer, 20, win_h - 80, panel_w - 40, 60,
                                 0x0A1428, 120);
    char buf[64];
    snprintf(buf, sizeof(buf), "IDENTIFIED: %s", game->current_profile->name);
    civ_font_render_aligned(renderer, font_profile, buf, 40, win_h - 80,
                            panel_w - 80, 60, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  int menu_x = 20;
  int menu_y = 220;
  int gap = 12;
  int btn_h = 48;
  int btn_w = panel_w - 40;

  btn_continue.x = menu_x;
  btn_continue.y = menu_y;
  btn_continue.w = btn_w;
  btn_continue.h = btn_h;

  btn_new_game.x = menu_x;
  btn_new_game.y = menu_y + (btn_h + gap);
  btn_new_game.w = btn_w;
  btn_new_game.h = btn_h;

  btn_load_game.x = menu_x;
  btn_load_game.y = menu_y + 2 * (btn_h + gap);
  btn_load_game.w = btn_w;
  btn_load_game.h = btn_h;

  btn_logout.x = menu_x;
  btn_logout.y = menu_y + 4 * (btn_h + gap) + 40;
  btn_logout.w = btn_w;
  btn_logout.h = btn_h;

  btn_exit.x = menu_x;
  btn_exit.y = menu_y + 5 * (btn_h + gap) + 40;
  btn_exit.w = btn_w;
  btn_exit.h = btn_h;

  btn_continue.enabled = false;
  civ_button_render(renderer, font_button, &btn_continue, input);

  if (civ_button_render(renderer, font_button, &btn_new_game, input)) {
    civ_scene_manager_switch(SCENE_SETUP);
  }

  if (civ_button_render(renderer, font_button, &btn_load_game, input)) {
    if (game->current_profile) {
      char **save_slots = NULL;
      int save_count =
          civ_profile_list_saves(game->current_profile->id, &save_slots);

      if (save_count > 0) {
        char save_path[256];
        if (civ_profile_get_save_path(game->current_profile->id, save_slots[0],
                                      save_path, sizeof(save_path))) {
          civ_result_t res = civ_game_load(game, save_path);
          if (res.error == CIV_OK) {
            printf("Loaded slot '%s' for %s. Entering game.\n", save_slots[0],
                   game->current_profile->name);
            civ_scene_manager_switch(SCENE_GAME);
          } else {
            printf("Failed to load game: %s\n", res.message);
          }
        }
      } else {
        printf("No save slots found for %s\n", game->current_profile->name);
      }
      civ_profile_free_list(save_slots, save_count);
    }
  }

  if (civ_button_render(renderer, font_button, &btn_logout, input)) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }

  if (civ_button_render(renderer, font_button, &btn_exit, input)) {
    SDL_Event quit_event;
    quit_event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit_event);
  }
}

static void destroy(void) {
  if (font_title)
    civ_font_destroy(font_title);
  if (font_profile)
    civ_font_destroy(font_profile);
  if (font_button)
    civ_font_destroy(font_button);

  font_title = NULL;
  font_profile = NULL;
  font_button = NULL;
}

civ_scene_t scene_main_menu = {.init = init,
                               .update = update,
                               .render = render,
                               .destroy = destroy,
                               .next_scene_id = -1};
