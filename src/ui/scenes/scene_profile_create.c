/**
 * @file scene_profile_create.c
 * @brief Scene for creating a new player profile
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
static civ_font_t *font_label = NULL;
static civ_font_t *font_input = NULL;

static char input_buffer[32] = "";
static civ_button_t btn_create;
static civ_button_t btn_cancel;

static bool text_input_started = false;
static SDL_Window *window_handle = NULL;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 48);
  font_label = civ_font_load_system("Segoe UI", 24);
  font_input = civ_font_load_system("Segoe UI", 32);

  input_buffer[0] = '\0';

  btn_create = civ_button_create(20, 0, 0, 200, 50, "CREATE");
  btn_create.color = CIV_COLOR_SUCCESS;

  btn_cancel = civ_button_create(21, 0, 0, 200, 50, "CANCEL");
  btn_cancel.color = CIV_COLOR_ERROR;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  /* Handle text input */
  if (input->text_input[0] != '\0') {
    size_t len = strlen(input_buffer);
    size_t new_text_len = strlen(input->text_input);
    if (len + new_text_len < 31) {
      strcat(input_buffer, input->text_input);
    }
  }

  if (input->backspace_pressed) {
    size_t len = strlen(input_buffer);
    if (len > 0) {
      input_buffer[len - 1] = '\0';
    }
  }

  if (input->esc_pressed) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  if (!text_input_started) {
    window_handle = SDL_GetRenderWindow(renderer);
    if (window_handle) {
      SDL_StartTextInput(window_handle);
      text_input_started = true;
    }
  }

  /* 1. Background */
  civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(renderer, 0, win_h - 400, win_w, 400,
                               CIV_COLOR_GLOW, 30);

  /* 2. Center Card Panel */
  int card_w = 480;
  int card_h = 420;
  int card_x = (win_w - card_w) / 2;
  int card_y = (win_h - card_h) / 2;

  civ_render_rect_filled_alpha(renderer, card_x, card_y, card_w, card_h,
                               CIV_COLOR_BG_MEDIUM, 200);
  civ_render_rect_outline(renderer, card_x, card_y, card_w, card_h, 0x2A3A4A,
                          1);

  /* 3. Title */
  if (font_title) {
    civ_font_render_aligned(renderer, font_title, "INITIALIZE PROFILE", card_x,
                            card_y + 40, card_w, 60, CIV_COLOR_PRIMARY,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* 4. Input Label */
  if (font_label) {
    civ_font_render_aligned(renderer, font_label, "DESIGNATION:", card_x,
                            card_y + 110, card_w, 40, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* 5. Input Field */
  int box_w = 360;
  int box_h = 56;
  int box_x = card_x + (card_w - box_w) / 2;
  int box_y = card_y + 160;

  civ_render_rect_filled(renderer, box_x, box_y, box_w, box_h, 0x0A0F1E);
  civ_render_rect_outline(renderer, box_x, box_y, box_w, box_h,
                          CIV_COLOR_ACCENT, 1);

  if (font_input) {
    const char *display_text = (input_buffer[0] == '\0') ? "_" : input_buffer;
    civ_font_render_aligned(renderer, font_input, display_text, box_x + 10,
                            box_y, box_w - 20, box_h, CIV_COLOR_TEXT,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* 6. Buttons */
  int btn_w = 170;
  btn_create.x = card_x + 60;
  btn_create.y = card_y + 300;
  btn_create.w = btn_w;
  btn_create.h = 48;

  btn_cancel.x = card_x + card_w - 60 - btn_w;
  btn_cancel.y = card_y + 300;
  btn_cancel.w = btn_w;
  btn_cancel.h = 48;

  if (civ_button_render(renderer, font_label, &btn_create, input)) {
    if (strlen(input_buffer) > 0) {
      civ_player_profile_t *profile = civ_profile_create(input_buffer);
      if (profile) {
        civ_profile_save(profile);
        if (game->current_profile)
          civ_profile_destroy(game->current_profile);
        game->current_profile = profile;
        civ_scene_manager_switch(SCENE_MAIN_MENU);
      }
    }
  }

  if (civ_button_render(renderer, font_label, &btn_cancel, input)) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void destroy(void) {
  if (font_title)
    civ_font_destroy(font_title);
  if (font_label)
    civ_font_destroy(font_label);
  if (font_input)
    civ_font_destroy(font_input);

  if (window_handle) {
    SDL_StopTextInput(window_handle);
  }
  text_input_started = false;
  window_handle = NULL;
  font_label = NULL;
  font_input = NULL;
}

civ_scene_t scene_profile_create = {.init = init,
                                    .update = update,
                                    .render = render,
                                    .destroy = destroy,
                                    .next_scene_id = -1};
