/**
 * @file research_panel.c
 * @brief Technology domain index display — no tech tree, continuous indices
 */
#include "ui/panel/research_panel.h"
#include "core/technology/innovation_system.h"
#include "engine/renderer.h"
#include <stdio.h>

void civ_research_panel_render(SDL_Renderer *r, civ_game_t *game,
                               civ_font_t *font, civ_input_state_t *input,
                               int win_w, int win_h) {
  if (!r || !game || !game->technology_tree || !font) return;

  civ_innovation_system_t *is = game->technology_tree;
  int pw = 620, ph = 480;
  int px = (win_w - pw) / 2, py = (win_h - ph) / 2;

  civ_render_rect_filled_alpha(r, px, py, pw, ph, 0x0A0F1A, 250);
  civ_render_rect_outline(r, px, py, pw, ph, 0x00A0FF, 2);
  civ_render_line(r, px, py + 44, px + pw, py + 44, 0x1A2A3A);

  civ_font_render_aligned(r, font, "TECHNOLOGY INDICES", px + 20, py,
                          pw - 40, 44, 0xFFFFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  /* Aggregate */
  char buf[128];
  sprintf(buf, "Aggregate Innovation Index: %+d  |  Budget: +%.1f/YR",
          is->aggregate_index, is->total_budget);
  civ_font_render_aligned(r, font, buf, px + 20, py + 50, pw - 40, 22,
                          0x00A0FF, CIV_ALIGN_CENTER, CIV_VALIGN_TOP);

  /* Divider */
  int curr_y = py + 80;
  civ_render_line(r, px + 20, curr_y, px + pw - 20, curr_y, 0x1A2A3A);
  curr_y += 12;

  /* Domain list */
  int cols = 2, items_per_col = CIV_TECH_DOMAIN_COUNT / cols;
  int col_w = (pw - 60) / cols;

  for (int i = 0; i < CIV_TECH_DOMAIN_COUNT; i++) {
    int col = i / items_per_col;
    int row = i % items_per_col;
    int dx = px + 20 + col * col_w;
    int dy = curr_y + row * 32;

    civ_tech_domain_state_t *d = &is->domains[i];

    /* Domain name */
    civ_font_render_aligned(r, font, d->name, dx, dy, col_w - 120, 18,
                            0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    /* Index value with sign */
    sprintf(buf, "%+d", d->index);
    uint32_t ic = d->index > 0  ? 0x44FF44
                  : d->index < 0 ? 0xFF4444
                                 : 0x888888;
    civ_font_render_aligned(r, font, buf, dx + col_w - 120, dy, 60, 18,
                            ic, CIV_ALIGN_RIGHT, CIV_VALIGN_TOP);

    /* Growth rate */
    sprintf(buf, "+%.1f/t", d->growth_rate);
    civ_font_render_aligned(r, font, buf, dx + col_w - 55, dy, 50, 18,
                            0x667788, CIV_ALIGN_RIGHT, CIV_VALIGN_TOP);

    /* Global rank */
    sprintf(buf, "#%d", d->global_rank);
    civ_font_render_aligned(r, font, buf, dx + col_w - 55, dy + 14, 50, 14,
                            0x555555, CIV_ALIGN_RIGHT, CIV_VALIGN_TOP);
  }

  /* Help */
  civ_font_render_aligned(r, font, "[T TO CLOSE]", win_w / 2, py + ph - 30,
                          200, 20, 0x666666, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);
  (void)input;
}

bool civ_research_panel_click(civ_game_t *game, civ_input_state_t *input,
                              int win_w, int win_h) {
  (void)game; (void)input; (void)win_w; (void)win_h;
  return false;
}
