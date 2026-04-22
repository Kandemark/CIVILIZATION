/**
 * @file scene_life_origin.c
 * @brief Assessment-based life origin — your answers place you, you don't pick
 *
 * The player answers scenario questions. Each answer maps to skills.
 * After all questions, the closest background is assigned automatically.
 */
#include "core/character.h"
#include "core/profile.h"
#include "core/world/map_generator.h"
#include "display/camera.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static civ_font_t               *font_title = NULL;
static civ_font_t               *font_body = NULL;
static civ_font_t               *font_small = NULL;
static civ_render_map_context_t *map_ctx = NULL;
static civ_camera_t              cam;
static int                       last_win_w, last_win_h;
static int                       question_idx = 0;
static int                       scores[CIV_BG_COUNT];
static int                       result_bg = -1;
static float                     pulse;

/* Assessment questions */
static const char *questions[4] = {
    "A dispute erupts in your community over land rights. What do you do?",
    "Your family's fortunes have declined. How do you rebuild?",
    "A stranger arrives offering new ideas that challenge local traditions.",
    "You are given authority over a small group. How do you lead?",
};
static const char *answers[4][4] = {
    {"Work the land yourself — it belongs to those who tend it",
     "Negotiate a trade agreement benefiting all sides",
     "Enforce the boundary with strength — borders must be clear",
     "Study the legal precedents and apply the correct statute"},
    {"Diversify crops and sell surplus at distant markets",
     "Open a workshop producing goods the region lacks",
     "Join a military campaign — plunder rebuilds wealth fastest",
     "Seek a position in the administrative apparatus"},
    {"Learn from them — new knowledge is the path forward",
     "Trade with them — commerce transcends tradition",
     "Test their ideas against the old ways — let the stronger survive",
     "Bring them before the elders for proper evaluation"},
    {"Lead by example — work alongside them in the fields",
     "Organize them into efficient production units",
     "Command with clear hierarchy — discipline wins battles",
     "Establish fair rules and procedures for all to follow"},
};
/* Scoring: each answer column maps to primary backgrounds + skills */
static const int answer_weights[4][4][3] = {
    {{CIV_BG_PEASANT, CIV_SKILL_SURVIVAL, 3},
     {CIV_BG_MERCHANT, CIV_SKILL_DIPLOMACY, 3},
     {CIV_BG_MILITARY, CIV_SKILL_COMBAT, 3},
     {CIV_BG_ACADEMIC, CIV_SKILL_SCHOLARSHIP, 3}},
    {{CIV_BG_PEASANT, CIV_SKILL_STEWARDSHIP, 3},
     {CIV_BG_ARTISAN, CIV_SKILL_CRAFTSMANSHIP, 3},
     {CIV_BG_MILITARY, CIV_SKILL_COMBAT, 3},
     {CIV_BG_BUREAUCRATIC, CIV_SKILL_ADMINISTRATION, 3}},
    {{CIV_BG_ACADEMIC, CIV_SKILL_SCHOLARSHIP, 3},
     {CIV_BG_MERCHANT, CIV_SKILL_TRADE, 3},
     {CIV_BG_NOMADIC, CIV_SKILL_SURVIVAL, 3},
     {CIV_BG_BUREAUCRATIC, CIV_SKILL_ADMINISTRATION, 3}},
    {{CIV_BG_PEASANT, CIV_SKILL_LEADERSHIP, 3},
     {CIV_BG_ARTISAN, CIV_SKILL_CRAFTSMANSHIP, 3},
     {CIV_BG_MILITARY, CIV_SKILL_LEADERSHIP, 3},
     {CIV_BG_BUREAUCRATIC, CIV_SKILL_ADMINISTRATION, 3}},
};

static void compute_result(void) {
  int best_bg = CIV_BG_PEASANT, best_score = -1;
  for (int i = 0; i < CIV_BG_COUNT; i++) {
    if (scores[i] > best_score) { best_score = scores[i]; best_bg = i; }
  }
  /* Aristocratic bonus if multiple backgrounds scored */
  if (best_score < 5) best_bg = CIV_BG_ARISTOCRATIC;
  result_bg = best_bg;
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 22);
  font_body  = civ_font_load_system("Segoe UI", 14);
  font_small = civ_font_load_system("Segoe UI", 12);
  map_ctx = NULL; question_idx = 0; result_bg = -1; pulse = 0.0f;
  memset(scores, 0, sizeof(scores));
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  pulse += 0.016f;
  if (cam.map_width == 0 && game->world_map)
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  civ_camera_update(&cam, 0.016f);
  cam.target_x += 0.04f;

  if (input->esc_pressed && question_idx > 0) { question_idx--; return; }
  if (input->esc_pressed && question_idx == 0) {
    civ_scene_manager_switch(SCENE_SETUP); return;
  }

  /* Assessment phase */
  if (result_bg < 0 && input->mouse_left_pressed) {
    int qh = 300, qy = input->win_h - qh - 40;
    for (int a = 0; a < 4; a++) {
      int ay = qy + 10 + a * 55;
      if (input->mouse_x > 40 && input->mouse_x < last_win_w - 40 &&
          input->mouse_y > ay && input->mouse_y < ay + 48) {
        for (int w = 0; w < 3; w++)
          scores[answer_weights[question_idx][a][w]] +=
              answer_weights[question_idx][a][w + 1 > 2 ? 0 : 0];
        question_idx++;
        if (question_idx >= 4) compute_result();
        break;
      }
    }
  }

  /* Result confirmation */
  if (result_bg >= 0 && (input->enter_pressed || input->mouse_left_pressed)) {
    if (game && game->current_profile) {
      civ_character_t *pc = civ_character_create(game->current_profile->name);
      civ_character_apply_background(pc, (civ_background_t)result_bg);
      if (game->player_character)
        civ_character_destroy((civ_character_t *)game->player_character);
      game->player_character = pc;
      printf("[LIFE_ORIGIN] Assessed background: %s\n",
             civ_background_name((civ_background_t)result_bg));
    }
    civ_scene_manager_switch(SCENE_IDENTITY);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  last_win_w = win_w; last_win_h = win_h;

  if (game->world_map) {
    if (!map_ctx) {
      map_ctx = civ_render_map_context_create(r, win_w, win_h,
          game->world_map->width, game->world_map->height);
      if (map_ctx) { map_ctx->view_x = cam.x; map_ctx->view_y = cam.y; map_ctx->zoom = cam.zoom; }
    }
    if (map_ctx) { map_ctx->view_x = cam.x; map_ctx->view_y = cam.y; map_ctx->zoom = cam.zoom; }
    civ_render_map(r, map_ctx, game->world_map, win_w, win_h,
                   CIV_MAP_VIEW_POLITICAL, NULL);
  } else {
    civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  }
  civ_render_rect_filled_alpha(r, 0, 0, win_w, win_h, 0x000000, 140);

  /* Title */
  if (font_title) {
    civ_font_render_aligned(r, font_title,
        result_bg >= 0 ? "YOUR ORIGIN" : "WHO ARE YOU?",
        0, 20, win_w, 36, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  if (result_bg >= 0) {
    /* Show result */
    civ_background_t bg = (civ_background_t)result_bg;
    civ_font_render_aligned(r, font_body, civ_background_name(bg),
                            0, win_h/2 - 40, win_w, 30,
                            CIV_COLOR_ACCENT, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    civ_font_render_aligned(r, font_small, civ_background_description(bg),
                            40, win_h/2, win_w - 80, 60,
                            0x8899AA, CIV_ALIGN_CENTER, CIV_VALIGN_TOP);
    float cp = (sinf(pulse * 3.0f) * 0.5f + 0.5f);
    civ_font_render_aligned(r, font_body, "CLICK OR ENTER TO BEGIN YOUR LIFE",
                            0, win_h/2 + 60, win_w, 24,
                            cp > 0.5f ? CIV_COLOR_PRIMARY : 0x888888,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  } else {
    /* Show question */
    int qh = 300, qy = win_h - qh - 40;
    civ_render_rect_filled_alpha(r, 30, qy, win_w - 60, qh, 0x0A1220, 220);
    civ_render_rect_outline(r, 30, qy, win_w - 60, qh, 0x00A0FF, 1);

    /* Progress */
    char buf[64];
    snprintf(buf, sizeof(buf), "Question %d of 4", question_idx + 1);
    civ_font_render_aligned(r, font_small, buf, 40, qy + 5, 120, 18,
                            0x667788, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    /* Question text */
    civ_font_render_aligned(r, font_body, questions[question_idx],
                            40, qy + 30, win_w - 80, 40,
                            0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    /* Answer buttons */
    for (int a = 0; a < 4; a++) {
      int ay = qy + 80 + a * 55;
      bool hover = civ_input_is_mouse_over(input, 40, ay, win_w - 80, 48);
      uint32_t abg = hover ? 0x162033 : 0x0A1220;
      uint32_t ab = hover ? g_theme.primary : 0x1A2A3A;
      civ_render_rect_filled_alpha(r, 40, ay, win_w - 80, 48, abg, 200);
      civ_render_rect_outline(r, 40, ay, win_w - 80, 48, ab, 1);
      char lbl[4] = {'A','B','C','D'};
      snprintf(buf, sizeof(buf), "%c. %s", lbl[a], answers[question_idx][a]);
      civ_font_render_aligned(r, font_small, buf, 55, ay, win_w - 110, 48,
                              hover ? 0xFFFFFF : 0x8899AA,
                              CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    }

    /* Instruction */
    civ_font_render_aligned(r, font_small,
        "Your answers determine your background. There are no wrong choices.",
        0, qy + qh - 20, win_w, 16, 0x555555, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }
  (void)input;
}

static void destroy(void) {
  if (font_title) civ_font_destroy(font_title), font_title = NULL;
  if (font_body)  civ_font_destroy(font_body),  font_body = NULL;
  if (font_small) civ_font_destroy(font_small), font_small = NULL;
  if (map_ctx)    civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_life_origin = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
