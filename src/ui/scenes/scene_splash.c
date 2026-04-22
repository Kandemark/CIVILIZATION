#include "display/theme.h"
#include "engine/font.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include <SDL3/SDL.h>

static civ_font_t *font_title = NULL;
static civ_font_t *font_prompt = NULL;
static civ_font_t *font_version = NULL;
static float splash_timer = 0.0f;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 48);
  font_prompt = civ_font_load_system("Segoe UI", 16);
  font_version = civ_font_load_system("Segoe UI", 12);
  splash_timer = 0.0f;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  splash_timer += 0.016f;
  if (input->mouse_left_pressed || splash_timer > 5.0f) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  SDL_SetRenderDrawColor(r, 5, 8, 15, 255);
  SDL_RenderClear(r);

  /* Title */
  if (font_title) {
    /* Fade in over 1.5 seconds */
    float alpha = splash_timer / 1.5f;
    if (alpha > 1.0f) alpha = 1.0f;
    uint32_t c = civ_theme_mix(g_theme.bg_deep, CIV_COLOR_PRIMARY, alpha);
    civ_font_render_aligned(r, font_title, "DOMINION", 0, -40, win_w, win_h,
                            c, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* Subtitle */
  if (font_prompt && splash_timer > 1.5f) {
    float pulse = 0.6f + 0.4f * sinf(splash_timer * 2.5f);
    uint8_t a = (uint8_t)(pulse * 180.0f);
    uint32_t pc = g_theme.text_dim;
    civ_font_render_aligned(r, font_prompt, "CLICK OR PRESS ANY KEY TO BEGIN",
                            0, 40, win_w, 30, pc,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    (void)a; /* alpha on font render is via color dimming */
  }

  /* Version */
  if (font_version) {
    char buf[32];
    snprintf(buf, sizeof(buf), "v%s", game && game->config.version[0]
           ? game->config.version : "0.2.0");
    civ_font_render_aligned(r, font_version, buf,
        win_w - 80, win_h - 24, 70, 20,
        g_theme.text_dim, CIV_ALIGN_RIGHT, CIV_VALIGN_BOTTOM);
  }

  (void)input;
}

static void destroy(void) {
  if (font_title)   { civ_font_destroy(font_title); font_title = NULL; }
  if (font_prompt)  { civ_font_destroy(font_prompt); font_prompt = NULL; }
  if (font_version) { civ_font_destroy(font_version); font_version = NULL; }
}

civ_scene_t scene_splash = {
  .init = init, .update = update, .render = render,
  .destroy = destroy, .next_scene_id = -1
};
