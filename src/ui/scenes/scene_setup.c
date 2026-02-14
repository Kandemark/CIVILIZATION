#include "../../../include/core/world/map_generator.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/engine/window.h"
#include "../../../include/ui/button.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>

/* Local state */
static civ_font_t *font_body = NULL;
static civ_font_t *font_button = NULL;
static bool is_generating = false;
static bool gen_finished = false;
extern float g_gen_progress; /* from map_generator.c */

static int map_gen_thread(void *data) {
  civ_game_t *game = (civ_game_t *)data;
  printf("Map generation thread started...\n");
  if (game && game->world_map) {
    civ_map_genesis(game->world_map);
  }
  g_gen_progress = 1.0f;
  printf("Map generation thread finished.\n");
  gen_finished = true;
  return 0;
}

static void init(void) {
  font_body = civ_font_load_system("Segoe UI", 24);
  font_button = civ_font_load_system("Segoe UI", 20);
  is_generating = false;
  gen_finished = false;
  g_gen_progress = 0.0f;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (!is_generating && g_gen_progress < 0.1f) {
    is_generating = true;
    gen_finished = false;
    SDL_CreateThread(map_gen_thread, "MapGenThread", game);
  }

  if (gen_finished || g_gen_progress >= 1.0f) {
    if (input->mouse_left_pressed || input->enter_pressed) {
      printf("Proceeding to spawn selection.\n");
      civ_scene_manager_switch(SCENE_SPAWN_SELECT);
    }
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  /* 1. Atmospheric Background */
  civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);

  /* Background activity pulse (simulated) */
  float pulse = (sinf(SDL_GetTicks() * 0.002f) * 0.5f + 0.5f);
  civ_render_rect_filled_alpha(renderer, 0, win_h - 200, win_w, 200,
                               CIV_COLOR_GLOW, (uint8_t)(pulse * 30));

  /* 2. Center Status Card */
  int card_w = 600;
  int card_h = 240;
  int card_x = (win_w - card_w) / 2;
  int card_y = (win_h - card_h) / 2;

  civ_render_rect_filled_alpha(renderer, card_x, card_y, card_w, card_h,
                               CIV_COLOR_BG_MEDIUM, 160);
  civ_render_rect_outline(renderer, card_x, card_y, card_w, card_h, 0x1A2A3A,
                          1);

  /* 3. Status Text */
  const char *status = (g_gen_progress < 0.99f)
                           ? "CONSTRUCTING PLANETARY GEOGRAPHY"
                           : "REALITY STABILIZED";
  if (font_body) {
    civ_font_render_aligned(renderer, font_body, status, card_x, card_y + 60,
                            card_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER,
                            CIV_VALIGN_TOP);

    char percent_buf[16];
    snprintf(percent_buf, sizeof(percent_buf), "%d%%",
             (int)(g_gen_progress * 100));
    civ_font_render_aligned(renderer, font_body, percent_buf, card_x,
                            card_y + 100, card_w, 30, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_CENTER, CIV_VALIGN_TOP);
  }

  /* 4. Progress Bar */
  int pw = 400;
  int ph = 8;
  int px = card_x + (card_w - pw) / 2;
  int py = card_y + 150;

  civ_render_rect_filled(renderer, px, py, pw, ph, 0x0A0F1E);
  civ_render_rect_filled(renderer, px, py, (int)(pw * g_gen_progress), ph,
                         CIV_COLOR_ACCENT);

  /* Glow bar */
  if (g_gen_progress > 0.01f) {
    civ_render_rect_filled_alpha(renderer, px, py - 2,
                                 (int)(pw * g_gen_progress), ph + 4,
                                 CIV_COLOR_ACCENT, 40);
  }

  /* 5. Completion Prompt */
  if (gen_finished || g_gen_progress >= 0.999f) {
    if (font_body) {
      uint8_t alpha =
          (uint8_t)((sinf(SDL_GetTicks() * 0.005f) * 0.5f + 0.5f) * 255);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
      civ_font_render_aligned(renderer, font_body, "PRESS TO INITIATE DESCENT",
                              0, py + 50, win_w, 40, CIV_COLOR_TEXT,
                              CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    }
  }
}

static void destroy(void) {
  if (font_body)
    civ_font_destroy(font_body);
  if (font_button)
    civ_font_destroy(font_button);
  font_body = NULL;
  font_button = NULL;
}

civ_scene_t scene_setup = {.init = init,
                           .update = update,
                           .render = render,
                           .destroy = destroy,
                           .next_scene_id = -1};
