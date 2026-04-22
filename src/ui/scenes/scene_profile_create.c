#include "core/game.h"
#include "core/profile.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/icon/icon_atlas.h"
#include "ui/ui_common.h"
#include "ui/widget/button.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

static civ_font_t           *font_title = NULL;
static civ_font_t           *font_label = NULL;
static civ_font_t           *font_input = NULL;
static civ_widget_button_t  *btn_create = NULL;
static civ_widget_button_t  *btn_cancel = NULL;
static char                  input_buffer[32] = "";
static bool                  text_input_started = false;
static SDL_Window           *window_handle = NULL;
static int                   avatar_choice = 0; /* index into avatar_presets */
static const civ_icon_id_t   avatar_presets[8] = {
    CIV_ICON_PERSON, CIV_ICON_CROWN, CIV_ICON_SHIELD, CIV_ICON_SWORD,
    CIV_ICON_STAR,   CIV_ICON_BOOK,  CIV_ICON_FLAG,   CIV_ICON_GEAR};

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 48);
  font_label = civ_font_load_system("Segoe UI", 24);
  font_input = civ_font_load_system("Segoe UI", 32);
  input_buffer[0] = '\0';
  btn_create = civ_widget_button_create("create_btn", 0, 0, 200, 50, "CREATE");
  btn_create->color_bg = 0x003A2A;
  btn_create->color_hover = 0x005544;
  btn_cancel = civ_widget_button_create("cancel_btn", 0, 0, 200, 50, "CANCEL");
  btn_cancel->color_bg = 0x3A0000;
  btn_cancel->color_hover = 0x550000;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  (void)game;
  /* Text input */
  if (input->text_input[0] != '\0') {
    size_t len = strlen(input_buffer);
    size_t nl = strlen(input->text_input);
    if (len + nl < 31) strcat(input_buffer, input->text_input);
  }
  if (input->backspace_pressed) {
    size_t len = strlen(input_buffer);
    if (len > 0) input_buffer[len - 1] = '\0';
  }
  if (input->esc_pressed) civ_scene_manager_switch(SCENE_PROFILE_SELECT);

  float dt = 1.0f / 60.0f;
  civ_widget_button_update(btn_create, input, dt);
  civ_widget_button_update(btn_cancel, input, dt);

  if (civ_widget_button_was_clicked(btn_create) && strlen(input_buffer) > 0) {
    civ_player_profile_t *p = civ_profile_create(input_buffer);
    if (p) {
      snprintf(p->avatar_path, sizeof(p->avatar_path), "icon:%d", avatar_choice);
      civ_profile_save(p);
      if (game->current_profile) civ_profile_destroy(game->current_profile);
      game->current_profile = p;
      civ_scene_manager_switch(SCENE_MAIN_MENU);
    }
  }
  if (civ_widget_button_was_clicked(btn_cancel))
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  (void)input;
  if (!text_input_started) {
    window_handle = SDL_GetRenderWindow(r);
    if (window_handle) SDL_StartTextInput(window_handle), text_input_started = true;
  }

  civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  civ_render_rect_filled_alpha(r, 0, win_h - 400, win_w, 400, CIV_COLOR_GLOW, 30);

  int card_w = 440, card_h = 380;
  int card_x = (win_w - card_w) / 2, card_y = (win_h - card_h) / 2;

  civ_render_rect_filled_alpha(r, card_x, card_y, card_w, card_h,
                               CIV_COLOR_BG_MEDIUM, 200);
  civ_render_rect_outline(r, card_x, card_y, card_w, card_h, 0x2A3A4A, 1);

  if (font_title)
    civ_font_render_aligned(r, font_title, "CREATE PROFILE", card_x,
                            card_y + 30, card_w, 46, CIV_COLOR_PRIMARY,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  civ_render_line(r, card_x+30, card_y+76, card_x+card_w-30, card_y+76, 0x1A2A3A);

  /* Name input */
  civ_font_render_aligned(r, font_label, "YOUR NAME", card_x+40, card_y+90,
                          120, 20, CIV_COLOR_TEXT_DIM, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  int box_w = card_w-80, box_h = 44;
  int box_x = card_x + 40, box_y = card_y + 116;

  civ_render_rect_filled(r, box_x, box_y, box_w, box_h, 0x080C18);
  civ_render_rect_outline(r, box_x, box_y, box_w, box_h,
                          input_buffer[0] ? CIV_COLOR_PRIMARY : 0x1A2A3A, 1);

  if (font_input) {
    const char *dt = (input_buffer[0] == '\0') ? "Enter your name..." : input_buffer;
    uint32_t tc = input_buffer[0] ? CIV_COLOR_TEXT : 0x445566;
    civ_font_render_aligned(r, font_input, dt, box_x + 12, box_y, box_w - 24,
                            box_h, tc, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  /* Avatar picker row */
  int avatar_y = card_y + 185;
  civ_font_render_aligned(r, font_label, "CHOOSE AVATAR", card_x+40, avatar_y-22,
                          160, 20, CIV_COLOR_TEXT_DIM, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  int av_size = 32, av_gap = 8;
  int av_total_w = 8 * av_size + 7 * av_gap;
  int av_start_x = card_x + 40 + 160; /* right-aligned with input */
  for (int i = 0; i < 8; i++) {
    int ax = av_start_x + i * (av_size + av_gap);
    bool hover = civ_input_is_mouse_over(input, ax, avatar_y, av_size, av_size);
    bool selected = (avatar_choice == i);

    uint32_t bg = selected ? 0x004A7A : (hover ? 0x1A2A3A : 0x0A1220);
    civ_render_rect_filled(r, ax - 2, avatar_y - 2, av_size + 4, av_size + 4, bg);
    civ_render_rect_outline(r, ax - 2, avatar_y - 2, av_size + 4, av_size + 4,
                            selected ? CIV_COLOR_PRIMARY : 0x1A2A3A, 1);
    /* Render icon as a colored box placeholder — real icon rendering
     * will use the SDF atlas when available */
    uint32_t ic = selected ? CIV_COLOR_PRIMARY : CIV_COLOR_TEXT_DIM;
    civ_render_rect_filled(r, ax + 4, avatar_y + 4, av_size - 8, av_size - 8, ic);
    civ_render_rect_outline(r, ax + 4, avatar_y + 4, av_size - 8, av_size - 8,
                            0xFFFFFF, 1);

    if (hover && input->mouse_left_pressed) avatar_choice = i;
  }

  int bw = 160;
  btn_create->base.x = (float)(card_x + 40);
  btn_create->base.y = (float)(card_y + card_h - 62);
  btn_create->base.w = (float)bw;
  btn_create->base.h = 48;
  btn_cancel->base.x = (float)(card_x + card_w - bw - 40);
  btn_cancel->base.y = (float)(card_y + 300);
  btn_cancel->base.w = (float)bw;
  btn_cancel->base.h = 48;

  civ_widget_button_render(btn_create, r, font_label);
  civ_widget_button_render(btn_cancel, r, font_label);
  (void)game;
}

static void destroy(void) {
  if (font_title)  civ_font_destroy(font_title),  font_title = NULL;
  if (font_label)  civ_font_destroy(font_label),  font_label = NULL;
  if (font_input)  civ_font_destroy(font_input),  font_input = NULL;
  if (window_handle) SDL_StopTextInput(window_handle);
  civ_widget_button_destroy(btn_create);
  civ_widget_button_destroy(btn_cancel);
  text_input_started = false;
  window_handle = NULL;
}

civ_scene_t scene_profile_create = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
