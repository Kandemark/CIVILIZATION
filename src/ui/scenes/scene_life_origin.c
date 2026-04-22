/**
 * @file scene_life_origin.c
 * @brief Assessment-based life origin — widget-based answer buttons
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
#include "ui/widget/button.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static civ_font_t               *font_title = NULL;
static civ_font_t               *font_body = NULL;
static civ_render_map_context_t *map_ctx = NULL;
static civ_camera_t              cam;
static int                       last_win_w, last_win_h;
static int                       question_idx = 0;
static int                       scores[CIV_BG_COUNT];
static int                       result_bg = -1;
static float                     pulse;
static civ_widget_button_t      *answer_btns[4];
static civ_widget_button_t      *btn_confirm_result = NULL;

static const char *questions[4] = {
    "A dispute erupts in your community over land rights. What do you do?",
    "Your family's fortunes have declined. How do you rebuild?",
    "A stranger arrives offering new ideas that challenge local traditions.",
    "You are given authority over a small group. How do you lead?",
};
static const char *answers[4][4] = {
    {"Work the land — it belongs to those who tend it",
     "Negotiate a trade agreement benefiting all sides",
     "Enforce the boundary with strength — borders are clear",
     "Study the legal precedents and apply the correct statute"},
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
     {CIV_BG_ARTISAN, CIV_SKILL_LEADERSHIP, 3},
     {CIV_BG_MILITARY, CIV_SKILL_LEADERSHIP, 3},
     {CIV_BG_BUREAUCRATIC, CIV_SKILL_LEADERSHIP, 3}},
};

static void compute_result(void) {
  int best = 0, best_bg = CIV_BG_PEASANT;
  for (int i = 0; i < CIV_BG_COUNT; i++)
    if (scores[i] > best) { best = scores[i]; best_bg = i; }
  if (best < 5) best_bg = CIV_BG_ARISTOCRATIC;
  result_bg = best_bg;
}

static void create_answer_buttons(void) {
  for (int i = 0; i < 4; i++) {
    char id[16], label[16];
    snprintf(id, sizeof(id), "ans_%d", i);
    snprintf(label, sizeof(label), "%c.", 'A' + i);
    if (!answer_btns[i])
      answer_btns[i] = civ_widget_button_create(id, 0, 0, 0, 44, "");
  }
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 24);
  font_body  = civ_font_load_system("Segoe UI", 14);
  question_idx = 0; result_bg = -1; pulse = 0.0f;
  memset(scores, 0, sizeof(scores));
  memset(answer_btns, 0, sizeof(answer_btns));
  btn_confirm_result = NULL;
  map_ctx = NULL;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  pulse += 0.016f;
  if (cam.map_width == 0 && game->world_map)
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  civ_camera_update(&cam, 0.016f);
  cam.target_x += 0.04f;

  if (input->esc_pressed) {
    if (result_bg >= 0) return;
    if (question_idx > 0) { question_idx--; return; }
    civ_scene_manager_switch(SCENE_SETUP); return;
  }

  /* Assessment phase — answer buttons */
  if (result_bg < 0) {
    create_answer_buttons();
    int qh = 280, qy = input->win_h - qh - 40;
    for (int a = 0; a < 4; a++) {
      if (!answer_btns[a]) continue;
      answer_btns[a]->base.x = 60.0f;
      answer_btns[a]->base.y = (float)(qy + 14 + a * 56);
      answer_btns[a]->base.w = (float)(last_win_w > 120 ? last_win_w - 120 : 600);
      answer_btns[a]->base.h = 44.0f;

      /* Update button text with letter prefix + answer */
      char full[256];
      snprintf(full, sizeof(full), "%c.  %s", 'A' + a, answers[question_idx][a]);
      civ_widget_button_set_text(answer_btns[a], full);

      civ_widget_button_update(answer_btns[a], input, 0.016f);

      if (civ_widget_button_was_clicked(answer_btns[a])) {
        for (int w = 0; w < 3; w++)
          scores[answer_weights[question_idx][a][w]] +=
              answer_weights[question_idx][a][w];
        question_idx++;
        if (question_idx >= 4) compute_result();
        break;
      }
    }
  }

  /* Result confirmation */
  if (result_bg >= 0) {
    if (!btn_confirm_result)
      btn_confirm_result = civ_widget_button_create("confirm_result",
          0, 0, 260, 40, "BEGIN YOUR LIFE");
    btn_confirm_result->base.x = (float)((last_win_w - 260) / 2);
    btn_confirm_result->base.y = (float)(last_win_h - 120);
    civ_widget_button_update(btn_confirm_result, input, 0.016f);
    if (civ_widget_button_was_clicked(btn_confirm_result) || input->enter_pressed) {
      if (game && game->current_profile) {
        civ_character_t *pc = civ_character_create(game->current_profile->name);
        civ_character_apply_background(pc, (civ_background_t)result_bg);
        if (game->player_character)
          civ_character_destroy((civ_character_t *)game->player_character);
        game->player_character = pc;
        printf("[LIFE_ORIGIN] Background: %s\n",
               civ_background_name((civ_background_t)result_bg));
      }
      civ_scene_manager_switch(SCENE_IDENTITY);
    }
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
    /* Dim overlay */
    civ_render_rect_filled_alpha(r, 0, 0, win_w, win_h, 0x000000, 140);
  } else {
    civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  }

  if (!font_title) return;

  if (result_bg < 0) {
    /* Question panel */
    int qh = 280, qy = win_h - qh - 40;
    civ_render_rect_filled_alpha(r, 20, qy, win_w - 40, qh, g_theme.panel_bg, 235);
    civ_render_rect_outline(r, 20, qy, win_w - 40, qh, g_theme.info, 1);

    /* Progress */
    char prog[32];
    snprintf(prog, sizeof(prog), "Question %d of 4", question_idx + 1);
    civ_font_render_aligned(r, font_title, prog, 40, qy + 10, 200, 28,
        g_theme.info, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    /* Progress bar */
    float pct = (float)(question_idx + 1) / 4.0f;
    civ_render_rect_filled(r, 40, qy + 40, win_w - 80, 4, g_theme.bg_medium);
    civ_render_rect_filled(r, 40, qy + 40, (int)((float)(win_w - 80) * pct), 4,
        g_theme.info);

    /* Question text */
    if (font_body)
      civ_font_render_aligned(r, font_body, questions[question_idx],
          40, qy + 52, win_w - 80, 30, g_theme.text_primary,
          CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    /* Answer buttons */
    for (int i = 0; i < 4; i++)
      if (answer_btns[i]) civ_widget_button_render(answer_btns[i], r, font_body);

    /* Hint */
    civ_font_render_aligned(r, font_body,
        "Your answers determine your background. There are no wrong choices.",
        40, qy + qh - 22, win_w - 80, 18, g_theme.text_dim,
        CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  } else {
    /* Result */
    const char *bg_name = civ_background_name((civ_background_t)result_bg);
    civ_font_render_aligned(r, font_title, "YOUR ORIGIN", 0, win_h/2 - 60,
        win_w, 36, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    civ_font_render_aligned(r, font_title, bg_name, 0, win_h/2 - 10,
        win_w, 40, CIV_COLOR_ACCENT, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

    float pulse_val = 0.5f + 0.5f * sinf(pulse * 3.0f);
    uint32_t pc = civ_theme_mix(g_theme.text_dim, g_theme.text_primary, pulse_val);
    civ_font_render_aligned(r, font_body, "CLICK OR PRESS ENTER TO BEGIN",
        0, win_h/2 + 50, win_w, 24, pc, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

    if (btn_confirm_result)
      civ_widget_button_render(btn_confirm_result, r, font_body);
  }
  (void)input;
}

static void destroy(void) {
  if (font_title) civ_font_destroy(font_title), font_title = NULL;
  if (font_body)  civ_font_destroy(font_body),  font_body = NULL;
  for (int i = 0; i < 4; i++)
    if (answer_btns[i]) civ_widget_button_destroy(answer_btns[i]), answer_btns[i] = NULL;
  if (btn_confirm_result) civ_widget_button_destroy(btn_confirm_result), btn_confirm_result = NULL;
  if (map_ctx) civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_life_origin = {.init = init, .update = update, .render = render,
                                 .destroy = destroy, .next_scene_id = -1};
