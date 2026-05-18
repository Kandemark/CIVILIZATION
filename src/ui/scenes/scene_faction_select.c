/**
 * @file scene_faction_select.c
 * @brief Faction identity selection — choose starting position on world indices
 *
 * 8 factions displayed as cards. Each shows starting index values (not
 * bonuses — there are no permanent modifiers), economic sector balance,
 * and initial governance power distribution.
 */
#include "../../../include/core/faction.h"
#include "../../../include/display/theme.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>

static civ_font_t *font_title = NULL;
static civ_font_t *font_card  = NULL;
static civ_font_t *font_small = NULL;
static int         hovered_idx = -1;
static int         selected_idx = -1;
static float       pulse;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 36);
  font_card  = civ_font_load_system("Segoe UI", 17);
  font_small = civ_font_load_system("Segoe UI", 12);
  hovered_idx = -1; selected_idx = -1; pulse = 0.0f;
}

static void render_index_bar(SDL_Renderer *r, int x, int y, int w,
                             const char *label, int32_t value, int32_t global_avg,
                             uint32_t color) {
  /* Show value relative to global average (centered on global_avg) */
  civ_font_render_aligned(r, font_small, label, x, y, 95, 12, 0x888888,
                          CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  int bar_w = w - 100;
  int bar_x = x + 100;
  int bar_y = y + 2, bar_h = 6;

  /* Scale: center = global_avg, range ±600 */
  int32_t range = 600;
  int32_t min_val = global_avg - range;
  int32_t offset = value - min_val;
  float pct = (float)offset / (float)(range * 2);
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 1.0f) pct = 1.0f;

  civ_render_rect_filled(r, bar_x, bar_y, bar_w, bar_h, 0x1A2A3A);
  /* Center marker */
  float center_pct = (float)(global_avg - min_val) / (float)(range * 2);
  civ_render_line(r, bar_x + (int)((float)bar_w * center_pct), bar_y - 1,
                  bar_x + (int)((float)bar_w * center_pct), bar_y + bar_h + 1,
                  0x333333);
  /* Value bar */
  int val_w = (int)((float)bar_w * pct);
  civ_render_rect_filled(r, bar_x, bar_y, val_w, bar_h, color);

  /* Value label */
  char buf[16];
  int32_t rel = value - global_avg;
  snprintf(buf, sizeof(buf), "%+d", rel);
  civ_font_render_aligned(r, font_small, buf, bar_x + bar_w + 4, y, 40, 12,
                          rel >= 0 ? 0x44FF44 : 0xFF4444,
                          CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
}

static void render_faction_card(SDL_Renderer *r, const civ_faction_t *f,
                                int cx, int cy, int cw, int ch,
                                civ_input_state_t *input, int idx) {
  bool hover = civ_input_is_mouse_over(input, cx, cy, cw, ch);
  bool sel = (selected_idx == idx);
  if (hover) hovered_idx = idx;

  uint32_t bg = hover ? 0x162033 : (sel ? 0x0D2540 : 0x0A1220);
  uint32_t border = sel ? f->color : (hover ? f->color_accent : 0x1A2A3A);
  int bw = sel ? 2 : 1;

  civ_render_rect_filled_alpha(r, cx, cy, cw, ch, bg, 230);
  civ_render_rect_outline(r, cx, cy, cw, ch, border, bw);
  civ_render_rect_filled(r, cx, cy, cw, 3, f->color);

  /* Name */
  civ_font_render_aligned(r, font_card, f->name, cx + 10, cy + 8, cw - 20, 22,
                          f->color_accent, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  /* Description */
  civ_font_render_aligned(r, font_small, f->description, cx + 10, cy + 32,
                          cw - 20, 30, 0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  /* Starting index bars (relative to global average 200) */
  int sy = cy + 68;
  int32_t avg = 200;
  int bar_area_w = cw - 20;
  render_index_bar(r, cx + 10, sy, bar_area_w, "Tech", f->start.tech_index, avg, 0x44BBFF);
  sy += 14;
  render_index_bar(r, cx + 10, sy, bar_area_w, "Military", f->start.military_index, avg, 0xFF4444);
  sy += 14;
  render_index_bar(r, cx + 10, sy, bar_area_w, "Economic", f->start.economic_index, avg, 0xFFCC00);
  sy += 14;
  render_index_bar(r, cx + 10, sy, bar_area_w, "Culture", f->start.cultural_index, avg, 0xCC66FF);
  sy += 14;

  /* Power distribution */
  sy = cy + ch - 44;
  char buf[96];
  snprintf(buf, sizeof(buf), "Executive %.0f%%  Legislative %.0f%%  Judicial %.0f%%",
           f->start.executive_power * 100.0f,
           f->start.legislative_power * 100.0f,
           f->start.judicial_power * 100.0f);
  civ_font_render_aligned(r, font_small, buf, cx + 10, sy, cw - 20, 14,
                          0x667788, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

  /* Terrain + settlements */
  snprintf(buf, sizeof(buf), "%s · %d settlement%s · radius %.1f",
           f->start.preferred_terrain,
           f->start.starting_settlements,
           f->start.starting_settlements > 1 ? "s" : "",
           f->start.starting_territory_radius);
  civ_font_render_aligned(r, font_small, buf, cx + 10, sy + 14, cw - 20, 14,
                          0x44FF88, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  pulse += 0.016f; hovered_idx = -1;

  if (input->esc_pressed) { civ_scene_manager_switch(SCENE_SETUP); return; }

  if (selected_idx >= 0 && (input->enter_pressed || input->mouse_left_pressed)) {
    const civ_faction_t *f = civ_faction_get(selected_idx);
    if (f && game) {
      strncpy(game->faction_id, f->id, sizeof(game->faction_id) - 1);
      printf("[FACTION] Selected: %s\n", f->name);
    }
    civ_scene_manager_switch(SCENE_SPAWN_SELECT);
    return;
  }
  (void)game;
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  (void)game;
  civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);

  if (font_title) {
    civ_font_render_aligned(r, font_title, "CHOOSE YOUR STARTING POSITION",
                            0, 20, win_w, 44, CIV_COLOR_PRIMARY,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    civ_font_render_aligned(r, font_small,
                            "Each defines where you begin on the world's "
                            "indices. Everything after is your decisions.",
                            0, 62, win_w, 20, 0x667788, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  }

  /* 4×2 grid */
  int cols = 4, rows = 2;
  int card_w = (win_w - 60) / cols;
  int card_h = (win_h - 140) / rows;
  int grid_x = 20, grid_y = 90;

  for (int i = 0; i < CIV_FACTION_COUNT; i++) {
    int col = i % cols, row = i / cols;
    int cx = grid_x + col * (card_w + 10);
    int cy = grid_y + row * (card_h + 10);

    if (input->mouse_left_pressed &&
        civ_input_is_mouse_over(input, cx, cy, card_w, card_h))
      selected_idx = i;

    render_faction_card(r, civ_faction_get(i), cx, cy, card_w, card_h, input, i);
  }

  if (selected_idx >= 0) {
    const civ_faction_t *f = civ_faction_get(selected_idx);
    float cp = (sinf(pulse * 3.0f) * 0.5f + 0.5f);
    char buf[128];
    snprintf(buf, sizeof(buf), "SELECTED: %s — ENTER OR CLICK TO CONFIRM",
             f ? f->name : "");
    civ_font_render_aligned(r, font_card, buf, 0, win_h - 32, win_w, 24,
                            cp > 0.5f ? (f ? f->color_accent : 0xFFFFFF) : 0x888888,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }
}

static void destroy(void) {
  if (font_title) civ_font_destroy(font_title), font_title = NULL;
  if (font_card)  civ_font_destroy(font_card),  font_card = NULL;
  if (font_small) civ_font_destroy(font_small), font_small = NULL;
}

civ_scene_t scene_faction_select = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
