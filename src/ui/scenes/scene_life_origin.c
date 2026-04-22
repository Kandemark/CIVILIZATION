/**
 * @file scene_life_origin.c
 * @brief Assessment-based life origin — Nuklear UI
 */
#include "core/character.h"
#include "core/profile.h"
#include "core/world/map_generator.h"
#include "display/camera.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

static civ_render_map_context_t *map_ctx = NULL;
static civ_camera_t              cam;
static int  last_win_w, last_win_h, question_idx, scores[CIV_BG_COUNT], result_bg;
static const char *questions[4] = {
    "A dispute erupts over land rights. What do you do?",
    "Your family's fortunes have declined. How do you rebuild?",
    "A stranger arrives offering ideas that challenge traditions.",
    "You are given authority over a small group. How do you lead?",
};
static const char *answers[4][4] = {
    {"Work the land — it belongs to those who tend it",
     "Negotiate a trade agreement benefiting all sides",
     "Enforce the boundary with strength — borders are clear",
     "Study the legal precedents and apply the statute"},
    {"Diversify crops and sell surplus at distant markets",
     "Open a workshop producing goods the region lacks",
     "Join a military campaign — plunder rebuilds wealth",
     "Seek a position in the administrative apparatus"},
    {"Learn from them — new knowledge is the path forward",
     "Trade with them — commerce transcends tradition",
     "Test their ideas against the old — let the stronger survive",
     "Bring them before the elders for proper evaluation"},
    {"Lead by example — work alongside them in the fields",
     "Organize them into efficient production units",
     "Command with clear hierarchy — discipline wins battles",
     "Establish fair rules and procedures for all to follow"},
};
static const int weights[4][4][3] = {
    {{CIV_BG_PEASANT,CIV_SKILL_SURVIVAL,3},{CIV_BG_MERCHANT,CIV_SKILL_DIPLOMACY,3},{CIV_BG_MILITARY,CIV_SKILL_COMBAT,3},{CIV_BG_ACADEMIC,CIV_SKILL_SCHOLARSHIP,3}},
    {{CIV_BG_PEASANT,CIV_SKILL_STEWARDSHIP,3},{CIV_BG_ARTISAN,CIV_SKILL_CRAFTSMANSHIP,3},{CIV_BG_MILITARY,CIV_SKILL_COMBAT,3},{CIV_BG_BUREAUCRATIC,CIV_SKILL_ADMINISTRATION,3}},
    {{CIV_BG_ACADEMIC,CIV_SKILL_SCHOLARSHIP,3},{CIV_BG_MERCHANT,CIV_SKILL_TRADE,3},{CIV_BG_NOMADIC,CIV_SKILL_SURVIVAL,3},{CIV_BG_BUREAUCRATIC,CIV_SKILL_ADMINISTRATION,3}},
    {{CIV_BG_PEASANT,CIV_SKILL_LEADERSHIP,3},{CIV_BG_ARTISAN,CIV_SKILL_LEADERSHIP,3},{CIV_BG_MILITARY,CIV_SKILL_LEADERSHIP,3},{CIV_BG_BUREAUCRATIC,CIV_SKILL_LEADERSHIP,3}},
};

static void compute_result(void) {
  int best = 0, best_bg = CIV_BG_PEASANT;
  for (int i = 0; i < CIV_BG_COUNT; i++)
    if (scores[i] > best) { best = scores[i]; best_bg = i; }
  if (best < 5) best_bg = CIV_BG_ARISTOCRATIC;
  result_bg = best_bg;
}

static void init(void) {
  question_idx = 0; result_bg = -1; memset(scores, 0, sizeof(scores)); map_ctx = NULL;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (cam.map_width == 0 && game->world_map)
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  civ_camera_update(&cam, 0.016f); cam.target_x += 0.04f;
  if (input->esc_pressed) {
    if (result_bg >= 0) return;
    if (question_idx > 0) { question_idx--; return; }
    civ_scene_manager_switch(SCENE_SETUP); return;
  }
  if (result_bg >= 0 && (input->enter_pressed || input->mouse_left_pressed)) {
    if (game->current_profile) {
      civ_character_t *pc = civ_character_create(game->current_profile->name);
      civ_character_apply_background(pc, (civ_background_t)result_bg);
      if (game->player_character) civ_character_destroy((civ_character_t*)game->player_character);
      game->player_character = pc;
    }
    civ_scene_manager_switch(SCENE_IDENTITY);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  last_win_w = win_w; last_win_h = win_h;
  if (game->world_map) {
    if (!map_ctx) {
      map_ctx = civ_render_map_context_create(r, win_w, win_h,
          game->world_map->width, game->world_map->height);
      if (map_ctx) { map_ctx->view_x = cam.x; map_ctx->view_y = cam.y; map_ctx->zoom = cam.zoom; }
    }
    if (map_ctx) { map_ctx->view_x = cam.x; map_ctx->view_y = cam.y; map_ctx->zoom = cam.zoom; }
    civ_render_map(r, map_ctx, game->world_map, win_w, win_h, CIV_MAP_VIEW_POLITICAL, NULL);
  }

  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  if (result_bg < 0) {
    /* Question panel */
    float pw = (float)win_w - 80, ph = 280, px = 40, py = (float)win_h - ph - 40;
    if (nk_begin(nk, "Assessment", nk_rect(px, py, pw, ph), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
      char buf[64];
      snprintf(buf, sizeof(buf), "Question %d of 4", question_idx + 1);
      nk_layout_row_dynamic(nk, 22, 1);
      nk_label(nk, buf, NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 18, 1);
      nk_label(nk, questions[question_idx], NK_TEXT_LEFT);
      nk_layout_row_dynamic(nk, 10, 1);
      for (int a = 0; a < 4; a++) {
        nk_layout_row_dynamic(nk, 34, 1);
        char label[256];
        snprintf(label, sizeof(label), "%c. %s", 'A' + a, answers[question_idx][a]);
        if (nk_button_label(nk, label)) {
          for (int w = 0; w < 3; w++)
            scores[weights[question_idx][a][w]] += weights[question_idx][a][w];
          question_idx++;
          if (question_idx >= 4) compute_result();
        }
      }
    }
    nk_end(nk);
  } else {
    /* Result */
    if (nk_begin(nk, "Result", nk_rect((float)(win_w-300)/2, (float)(win_h-180)/2, 300, 180),
                 NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
      nk_layout_row_dynamic(nk, 28, 1);
      nk_label(nk, "YOUR ORIGIN", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 32, 1);
      nk_label(nk, civ_background_name((civ_background_t)result_bg), NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 20, 1);
      nk_label(nk, "Click or press Enter to begin", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 38, 1);
      if (nk_button_label(nk, "BEGIN YOUR LIFE")) {
        if (game->current_profile) {
          civ_character_t *pc = civ_character_create(game->current_profile->name);
          civ_character_apply_background(pc, (civ_background_t)result_bg);
          if (game->player_character) civ_character_destroy((civ_character_t*)game->player_character);
          game->player_character = pc;
        }
        civ_scene_manager_switch(SCENE_IDENTITY);
      }
    }
    nk_end(nk);
  }
  (void)input; (void)win_w; (void)win_h;
}

static void destroy(void) {
  if (map_ctx) civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_life_origin = {.init = init, .update = update, .render = render,
                                 .destroy = destroy, .next_scene_id = -1};
