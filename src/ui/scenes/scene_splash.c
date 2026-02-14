#include "../../../include/engine/font.h"
#include "../../../include/engine/window.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>

/* Local state */
static civ_font_t *font_title = NULL;
static float splash_timer = 0.0f;
static float splash_alpha = 0.0f;

static void init(void) {
  /* Fonts should be loaded by the main system, but we can look them up or load
   * specific ones */
  /* For now, we'll assume the main app has loaded "Segoe UI" and we can reload
     it or share it. To keep it clean, let's load our own reference or rely on a
     font manager. For this refactor, we'll load a local handle. */
  int win_h = 1000; /* Default, will be updated in render */
  font_title = civ_font_load_system("Segoe UI", 80);
  splash_timer = 0.0f;
  splash_alpha = 0.0f;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  splash_timer += 0.016f; /* Approx 60fps */
  if (splash_timer < 2.0f) {
    splash_alpha = splash_timer / 2.0f;
  } else {
    splash_alpha = 1.0f;
  }

  if (input->mouse_left_pressed || splash_timer > 5.0f) {
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  civ_window_clear(NULL, 0x000000); /* We don't have window handle here easily,
                                       but renderer clears usually */
  /* Actually renderer needs a color to clear. civ_window_clear wrapper might
     not work without window struct. Let's use SDL_RenderClear with color. */
  SDL_SetRenderDrawColor(renderer, 5, 8, 15, 255);
  SDL_RenderClear(renderer);

  if (font_title) {
    uint8_t a = (uint8_t)(splash_alpha * 255.0f);
    SDL_SetRenderDrawColor(renderer, (CIV_COLOR_ACCENT >> 16) & 0xFF,
                           (CIV_COLOR_ACCENT >> 8) & 0xFF,
                           CIV_COLOR_ACCENT & 0xFF, a);

    /* We need a civ_font_render_aligned_alpha or similar if possible.
       For now we'll just use the accent color and hope the renderer respects
       SDL_SetRenderDrawColor alpha Wait, civ_font_render uses SDL_ttf which
       often manages its own color.
    */
    civ_font_render_aligned(renderer, font_title, "CIVILIZATION", 0, -20, win_w,
                            win_h, CIV_COLOR_ACCENT, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  }
}

static void destroy(void) {
  if (font_title) {
    civ_font_destroy(font_title);
    font_title = NULL;
  }
}

civ_scene_t scene_splash = {.init = init,
                            .update = update,
                            .render = render,
                            .destroy = destroy,
                            .next_scene_id = -1};
