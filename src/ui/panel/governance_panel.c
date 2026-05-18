/**
 * @file governance_panel.c
 * @brief Governance panel — dynamic political structure, no fixed types
 */
#include "../../../include/ui/panel/governance_panel.h"
#include "../../../include/core/governance/government.h"
#include "../../../include/engine/renderer.h"
#include <math.h>
#include <stdio.h>

void civ_governance_panel_render(SDL_Renderer *r, civ_game_t *game,
                                 civ_font_t *font, civ_input_state_t *input,
                                 int win_w, int win_h) {
  if (!r || !game || !game->government || !font) return;

  civ_government_t *gov = game->government;
  int pw = 520, ph = 480;
  int px = (win_w - pw) / 2, py = (win_h - ph) / 2;

  civ_render_rect_filled_alpha(r, px, py, pw, ph, 0x0D1525, 250);
  civ_render_rect_outline(r, px, py, pw, ph, 0xFFCC00, 2);
  civ_render_line(r, px, py + 44, px + pw, py + 44, 0x2A3A1A);

  civ_font_render_aligned(r, font, "NATIONAL GOVERNANCE", px + 20, py,
                          pw - 40, 30, 0xFFFFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  int curr_y = py + 40;
  char buf[128];

  /* Proximity label */
  const char *prox = civ_government_proximity_label(gov);
  sprintf(buf, "STRUCTURE: %s", prox);
  civ_font_render_aligned(r, font, buf, px + 20, curr_y, pw - 40, 18,
                          0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 22;

  sprintf(buf, "Authority: %.0f%%  Representation: %.0f%%  Rigidity: %.0f%%  Balance: %.0f%%",
          gov->profile.authority_concentration * 100.0f,
          gov->profile.representation_index * 100.0f,
          gov->profile.institutional_rigidity * 100.0f,
          gov->profile.power_balance * 100.0f);
  civ_font_render_aligned(r, font, buf, px + 20, curr_y, pw - 40, 14,
                          0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 20;

  /* Divider */
  civ_render_line(r, px + 20, curr_y, px + pw - 20, curr_y, 0x2A2A2A);
  curr_y += 8;

  /* Metrics */
  civ_font_render_aligned(r, font, "STABILITY", px + 24, curr_y, 100, 16,
                          0x888888, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(r, px + 140, curr_y + 4, 180, 8, 0x1A2A3A);
  civ_render_rect_filled(r, px + 140, curr_y + 4,
                         (int)(180.0f * gov->stability), 8, 0x00A0FF);
  curr_y += 24;

  civ_font_render_aligned(r, font, "LEGITIMACY", px + 24, curr_y, 100, 16,
                          0x888888, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(r, px + 140, curr_y + 4, 180, 8, 0x1A2A3A);
  civ_render_rect_filled(r, px + 140, curr_y + 4,
                         (int)(180.0f * gov->legitimacy), 8, 0x00FF88);
  curr_y += 24;

  civ_font_render_aligned(r, font, "EFFICIENCY", px + 24, curr_y, 100, 16,
                          0x888888, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(r, px + 140, curr_y + 4, 180, 8, 0x1A2A3A);
  civ_render_rect_filled(r, px + 140, curr_y + 4,
                         (int)(180.0f * gov->efficiency), 8, 0xFFCC00);
  curr_y += 24;

  /* Citizen happiness */
  sprintf(buf, "CITIZEN SATISFACTION: %.0f%%",
          gov->profile.citizen_happiness * 100.0f);
  uint32_t happy_color = gov->profile.citizen_happiness > 0.6f   ? 0x44FF44
                         : gov->profile.citizen_happiness > 0.3f ? 0xFFCC00
                                                                  : 0xFF4444;
  civ_font_render_aligned(r, font, buf, px + 24, curr_y, pw - 48, 18,
                          happy_color, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 24;

  /* Divider */
  civ_render_line(r, px + 20, curr_y, px + pw - 20, curr_y, 0x2A2A2A);
  curr_y += 8;

  /* Political positions tree */
  civ_font_render_aligned(r, font, "POLITICAL POSITIONS", px + 24, curr_y,
                          pw - 48, 20, 0xFFCC00, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 28;

  /* Scroll cutoff check — show up to 7 positions */
  int shown = 0, max_show = 7;
  for (size_t i = 0; i < gov->position_count && shown < max_show; i++) {
    civ_political_position_t *p = &gov->positions[i];
    int indent = p->hierarchy_level * 16;

    sprintf(buf, "%s", p->title);
    civ_font_render_aligned(r, font, buf, px + 24 + indent, curr_y, pw - 48 - indent,
                            18, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    sprintf(buf, "[%s · %s · %d seat%s]",
            p->selection_method, p->term, p->position_count,
            p->position_count > 1 ? "s" : "");
    civ_font_render_aligned(r, font, buf, px + 24 + indent, curr_y + 16,
                            pw - 48 - indent, 14, 0x666666, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);
    curr_y += 34;
    shown++;
  }
  if (gov->position_count > (size_t)max_show) {
    sprintf(buf, "... and %zu more positions",
            gov->position_count - (size_t)max_show);
    civ_font_render_aligned(r, font, buf, px + 24, curr_y, pw - 48, 16,
                            0x555555, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }

  /* Stature tier at bottom */
  const char *tier_names[] = {"Failed State", "Frontier Nation",
                              "Developing State", "Stable State",
                              "Regional Power", "Great Power", "Hegemon"};
  sprintf(buf, "Global Stature: %s  |  Governance Ranking: %.0f",
          tier_names[gov->stature_tier], gov->profile.governance_ranking);
  civ_font_render_aligned(r, font, buf, px + 24, py + ph - 24, pw - 48, 18,
                          0x00FFFF, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  (void)input;
}
