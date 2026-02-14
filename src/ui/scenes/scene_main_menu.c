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
  printf("Initializing Main Menu Scene\n");
  font_title = civ_font_load_system("Segoe UI", 64);
  font_profile = civ_font_load_system("Segoe UI", 20);
  font_button = civ_font_load_system("Segoe UI", 28);

  if (!font_title || !font_profile || !font_button) {
    printf("Warning: Some fonts failed to load in Main Menu\n");
  }

  btn_continue = civ_button_create(30, 0, 0, 300, 50, "CONTINUE");
  btn_new_game = civ_button_create(31, 0, 0, 300, 50, "NEW GAME");
  btn_load_game = civ_button_create(32, 0, 0, 300, 50, "LOAD GAME");
  btn_logout = civ_button_create(33, 0, 0, 300, 50, "SWITCH PROFILE");
  btn_exit = civ_button_create(34, 0, 0, 300, 50, "EXIT");

  /* Logout button style */
  btn_logout.color = 0x555555;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (input->esc_pressed) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  /* 1. Background atmospheric deep blue */
  civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);

  /* Add a subtle gradient or "glow" from the bottom left */
  civ_render_rect_filled_alpha(renderer, 0, win_h - 400, 600, 400,
                               CIV_COLOR_GLOW, 40);

  /* 2. Side Panel Glassmorphism */
  int panel_w = 340;
  civ_render_rect_filled_alpha(renderer, 0, 0, panel_w, win_h,
                               CIV_COLOR_BG_MEDIUM, 180);
  civ_render_line(renderer, panel_w, 0, panel_w, win_h, 0x1A2A3A);

  /* 3. Title (Top of Panel) */
  if (font_title) {
    civ_font_render_aligned(renderer, font_title, "CIVILIZATION", 40, 60,
                            panel_w - 80, 60, CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);
  }

  /* 4. Profile Section (Bottom of Panel) */
  if (font_profile && game->current_profile) {
    civ_render_rect_filled_alpha(renderer, 20, win_h - 80, panel_w - 40, 60,
                                 0x0A1428, 120);
    char buf[64];
    snprintf(buf, sizeof(buf), "IDENTIFIED: %s", game->current_profile->name);
    civ_font_render_aligned(renderer, font_profile, buf, 40, win_h - 80,
                            panel_w - 80, 60, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  /* 5. Menu Buttons */
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

  // Mocking continue availability
  btn_continue.enabled = false;

  if (civ_button_render(renderer, font_button, &btn_continue, input)) {
    // Continue logic
  }

  if (civ_button_render(renderer, font_button, &btn_new_game, input)) {
    civ_scene_manager_switch(SCENE_SETUP); // Setup is World Gen
  }

  if (civ_button_render(renderer, font_button, &btn_load_game, input)) {
    if (game->current_profile) {
      char save_path[256];
      snprintf(save_path, sizeof(save_path), "%s.civ",
               game->current_profile->name);
      civ_result_t res = civ_game_load(game, save_path);
      if (res.error == CIV_OK) {
        printf("Game state restored for %s. Transitioning to orbital view.\n",
               game->current_profile->name);
        civ_scene_manager_switch(SCENE_GAME);
      } else {
        printf("Failed to load game: %s\n", res.message);
      }
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
