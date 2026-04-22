#include "display/theme.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>

static float splash_timer = 0.0f;

static void init(void) { splash_timer = 0.0f; }

static void update(civ_game_t *game, civ_input_state_t *input) {
  splash_timer += 0.016f;
  if (input->mouse_left_pressed || input->enter_pressed || splash_timer > 5.0f)
    civ_scene_manager_switch(SCENE_PROFILE_SELECT);
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)r; (void)input;
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  float alpha = splash_timer / 1.5f; if (alpha > 1.0f) alpha = 1.0f;

  if (nk_begin(nk, "Splash", nk_rect(0, 0, (float)win_w, (float)win_h),
               NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_space_begin(nk, NK_STATIC, (float)win_h, 1);

    /* Title centered */
    nk_layout_space_push(nk, nk_rect(0, (float)win_h/2 - 50, (float)win_w, 60));
    nk_label(nk, "DOMINION", NK_TEXT_CENTERED);

    /* Prompt with pulse */
    if (splash_timer > 1.5f) {
      nk_layout_space_push(nk, nk_rect(0, (float)win_h/2 + 20, (float)win_w, 30));
      nk_label(nk, "Click or press any key to begin", NK_TEXT_CENTERED);
    }

    /* Version */
    nk_layout_space_push(nk, nk_rect((float)win_w - 120, (float)win_h - 30, 100, 22));
    nk_label(nk, "v0.2.0", NK_TEXT_RIGHT);

    nk_layout_space_end(nk);
  }
  nk_end(nk);
}

static void destroy(void) {}

civ_scene_t scene_splash = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
