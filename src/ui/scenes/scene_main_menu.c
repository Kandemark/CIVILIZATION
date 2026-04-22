/**
 * @file scene_main_menu.c
 * @brief Main menu — new widget-based implementation
 */
#include "core/game.h"
#include "core/profile.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include "ui/widget/button.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static civ_font_t              *font_title = NULL;
static civ_font_t              *font_profile = NULL;
static civ_font_t              *font_button = NULL;
static civ_widget_button_t     *btn_continue = NULL;
static civ_widget_button_t     *btn_new_game = NULL;
static civ_widget_button_t     *btn_load_game = NULL;
static civ_widget_button_t     *btn_logout = NULL;
static civ_widget_button_t     *btn_exit = NULL;
static civ_widget_button_t     *btn_quit = NULL;
static float                    dt;

static void layout_buttons(int panel_w) {
  int menu_x = 20, menu_y = 220, gap = 12, btn_h = 48, btn_w = panel_w - 40;
  civ_widget_button_set_position(btn_continue,  menu_x, menu_y);
  civ_widget_button_set_position(btn_new_game,   menu_x, menu_y + 1*(btn_h+gap));
  civ_widget_button_set_position(btn_load_game,  menu_x, menu_y + 2*(btn_h+gap));
  civ_widget_button_set_position(btn_logout,     menu_x, menu_y + 4*(btn_h+gap)+40);
  civ_widget_button_set_position(btn_exit,       menu_x, menu_y + 5*(btn_h+gap)+40);
  /* Resize */
  btn_continue->base.w = btn_new_game->base.w = btn_load_game->base.w = btn_w;
  btn_logout->base.w = btn_exit->base.w = btn_w;
  btn_continue->base.h = btn_new_game->base.h = btn_load_game->base.h = btn_h;
  btn_logout->base.h = btn_exit->base.h = btn_h;
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 28);
  font_profile = civ_font_load_system("Segoe UI", 14);
  font_button = civ_font_load_system("Segoe UI", 16);

  btn_continue  = civ_widget_button_create("menu_cont", 0,0,300,50,"CONTINUE");
  btn_new_game  = civ_widget_button_create("menu_new",  0,0,300,50,"NEW GAME");
  btn_load_game = civ_widget_button_create("menu_load", 0,0,300,50,"LOAD SLOT");
  btn_logout    = civ_widget_button_create("menu_logout",0,0,300,50,"SWITCH PROFILE");
  btn_exit      = civ_widget_button_create("menu_exit", 0,0,300,50,"EXIT TO DESKTOP");
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  (void)game;
  if (input->esc_pressed) civ_scene_manager_switch(SCENE_PROFILE_SELECT);

  /* Enable Continue if profile has saves */
  if (game->current_profile) {
    char **slots = NULL;
    int n = civ_profile_list_saves(game->current_profile->id, &slots);
    civ_widget_button_set_enabled(btn_continue, n > 0);
    civ_profile_free_list(slots, n);
  }

  dt = 1.0f / 60.0f; /* fixed step for menu */
  civ_widget_button_update(btn_continue, input, dt);
  civ_widget_button_update(btn_new_game, input, dt);
  civ_widget_button_update(btn_load_game, input, dt);
  civ_widget_button_update(btn_logout, input, dt);
  civ_widget_button_update(btn_exit, input, dt);

  if (civ_widget_button_was_clicked(btn_continue))
    civ_scene_manager_switch(SCENE_GAME);

  if (civ_widget_button_was_clicked(btn_new_game))
    civ_scene_manager_switch(SCENE_IDENTITY);

  if (civ_widget_button_was_clicked(btn_load_game) && game->current_profile) {
    char **slots = NULL;
    int n = civ_profile_list_saves(game->current_profile->id, &slots);
    if (n > 0) {
      char path[256];
      if (civ_profile_get_save_path(game->current_profile->id, slots[0], path,
                                    sizeof(path))) {
        civ_result_t r = civ_game_load(game, path);
        if (r.error == CIV_OK)
          civ_scene_manager_switch(SCENE_GAME);
        else
          printf("Load failed: %s\n", r.message);
      }
    }
    civ_profile_free_list(slots, n);
  }

  if (civ_widget_button_was_clicked(btn_logout))
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);

  if (civ_widget_button_was_clicked(btn_exit)) {
    SDL_Event e; e.type = SDL_EVENT_QUIT; SDL_PushEvent(&e);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  (void)input;

  civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(r, 0, win_h - 400, 600, 400, CIV_COLOR_GLOW, 40);

  int panel_w = 340;
  civ_render_rect_filled_alpha(r, 0, 0, panel_w, win_h, CIV_COLOR_BG_MEDIUM, 180);
  civ_render_line(r, panel_w, 0, panel_w, win_h, 0x1A2A3A);

  if (font_title)
    civ_font_render_aligned(r, font_title, "DOMINION", 40, 60, panel_w - 80, 60,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  if (font_profile && game->current_profile) {
    civ_render_rect_filled_alpha(r, 20, win_h - 80, panel_w - 40, 60, 0x0A1428, 120);
    char buf[64];
    snprintf(buf, sizeof(buf), "IDENTIFIED: %s", game->current_profile->name);
    civ_font_render_aligned(r, font_profile, buf, 40, win_h - 80, panel_w - 80,
                            60, CIV_COLOR_TEXT_DIM, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  layout_buttons(panel_w);

  civ_widget_button_render(btn_continue, r, font_button);
  civ_widget_button_render(btn_new_game, r, font_button);
  civ_widget_button_render(btn_load_game, r, font_button);
  civ_widget_button_render(btn_logout, r, font_button);
  civ_widget_button_render(btn_exit, r, font_button);
}

static void destroy(void) {
  civ_widget_button_destroy(btn_continue);
  civ_widget_button_destroy(btn_new_game);
  civ_widget_button_destroy(btn_load_game);
  civ_widget_button_destroy(btn_logout);
  civ_widget_button_destroy(btn_exit);
  if (font_title)   civ_font_destroy(font_title),   font_title = NULL;
  if (font_profile) civ_font_destroy(font_profile), font_profile = NULL;
  if (font_button)  civ_font_destroy(font_button),  font_button = NULL;
}

civ_scene_t scene_main_menu = {.init = init, .update = update, .render = render,
                               .destroy = destroy, .next_scene_id = -1};
