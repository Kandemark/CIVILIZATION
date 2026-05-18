#include "../../../include/ui/panel/diplomacy_panel.h"
#include "../../../include/core/diplomacy/relations.h"
#include "../../../include/engine/renderer.h"
#include <stdio.h>
#include <string.h>

void civ_diplomacy_panel_render(SDL_Renderer *r, civ_game_t *game,
                                civ_font_t *font, civ_input_state_t *input,
                                int win_w, int win_h, bool has_contacted_rival) {
  if (!r || !game || !font) return;

  int dsb_w = 280, dsb_h = 450, dsb_x = 20, dsb_y = 60;

  civ_render_rect_filled_alpha(r, dsb_x, dsb_y, dsb_w, dsb_h, 0x140505, 240);
  civ_render_rect_outline(r, dsb_x, dsb_y, dsb_w, dsb_h, 0xFFFF00, 1);
  civ_render_line(r, dsb_x, dsb_y + 40, dsb_x + dsb_w, dsb_y + 40, 0x3A1A1A);

  civ_font_render_aligned(r, font, "DIPLOMACY & RELATIONS", dsb_x + 15, dsb_y,
                          dsb_w - 30, 40, 0xFFFF00, CIV_ALIGN_LEFT,
                          CIV_VALIGN_MIDDLE);

  int curr_y = dsb_y + 55;
  if (has_contacted_rival) {
    civ_diplomatic_relation_t *rel =
        civ_diplomacy_system_get_relation(game->diplomacy_system, "player",
                                          "rival_kingdom");

    civ_font_render_aligned(r, font, "RIVAL KINGDOM", dsb_x + 15, curr_y,
                            dsb_w - 30, 30, 0xFFFFFF, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);
    curr_y += 25;

    if (rel) {
      char buf[64];
      const char *lvl_str = "Neutral";
      if (rel->relation_level == CIV_RELATION_LEVEL_WAR) lvl_str = "WAR";
      else if (rel->relation_level == CIV_RELATION_LEVEL_HOSTILE)
        lvl_str = "Hostile";
      else if (rel->relation_level == CIV_RELATION_LEVEL_FRIENDLY)
        lvl_str = "Friendly";
      else if (rel->relation_level == CIV_RELATION_LEVEL_ALLIED)
        lvl_str = "Allied";

      sprintf(buf, "STATUS: %s (Opinion: %.0f)", lvl_str, rel->opinion_score);
      civ_font_render_aligned(r, font, buf, dsb_x + 15, curr_y, dsb_w - 30, 30,
                              0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      curr_y += 25;

      const char *stance_str = "Neutral";
      uint32_t stance_color = 0xAAAAAA;
      if (rel->current_stance == CIV_STANCE_HOSTILE)
        stance_str = "HOSTILE", stance_color = 0xFF4444;
      else if (rel->current_stance == CIV_STANCE_WARY)
        stance_str = "WARY", stance_color = 0xFFCC00;
      else if (rel->current_stance == CIV_STANCE_FRIENDLY)
        stance_str = "FRIENDLY", stance_color = 0x44FF44;

      sprintf(buf, "STANCE: %s", stance_str);
      civ_font_render_aligned(r, font, buf, dsb_x + 15, curr_y, dsb_w - 30, 30,
                              stance_color, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      curr_y += 25;

      const char *p_str = "Balanced";
      if (rel->personality == CIV_PERSONALITY_AGGRESSIVE) p_str = "Aggressive";
      else if (rel->personality == CIV_PERSONALITY_EXPANSIONIST)
        p_str = "Expansionist";
      else if (rel->personality == CIV_PERSONALITY_MERCANTILE)
        p_str = "Mercantile";
      sprintf(buf, "PERSONALITY: %s", p_str);
      civ_font_render_aligned(r, font, buf, dsb_x + 15, curr_y, dsb_w - 30, 30,
                              0x00FFCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    }
    curr_y += 35;

    /* Action buttons */
    bool h_trade = civ_input_is_mouse_over(input, dsb_x + 10, curr_y,
                                           dsb_w - 20, 40);
    civ_render_rect_filled(r, dsb_x + 10, curr_y, dsb_w - 20, 40,
                           h_trade ? 0x2A3A4A : 0x1A2A3A);
    civ_font_render_aligned(r, font, "PROPOSE TRADE AGREEMENT", dsb_x + 10,
                            curr_y, dsb_w - 20, 40, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
    curr_y += 50;

    bool h_nap = civ_input_is_mouse_over(input, dsb_x + 10, curr_y,
                                         dsb_w - 20, 40);
    civ_render_rect_filled(r, dsb_x + 10, curr_y, dsb_w - 20, 40,
                           h_nap ? 0x2A3A4A : 0x1A2A3A);
    civ_font_render_aligned(r, font, "NON-AGGRESSION PACT", dsb_x + 10, curr_y,
                            dsb_w - 20, 40, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
    curr_y += 50;

    bool h_war = civ_input_is_mouse_over(input, dsb_x + 10, curr_y,
                                         dsb_w - 20, 40);
    civ_render_rect_filled(r, dsb_x + 10, curr_y, dsb_w - 20, 40,
                           h_war ? 0x660000 : 0x440000);
    civ_font_render_aligned(r, font, "DECLARE WAR", dsb_x + 10, curr_y,
                            dsb_w - 20, 40, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  } else {
    civ_font_render_aligned(r, font, "NO NATIONS DISCOVERED", dsb_x + 15, curr_y,
                            dsb_w - 30, 30, 0x666666, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);
  }
}

bool civ_diplomacy_panel_click(civ_game_t *game, civ_input_state_t *input,
                               int win_w, int win_h, bool has_contacted_rival) {
  if (!game || !input || !has_contacted_rival) return false;

  int dsb_x = 20, dsb_y = 60, dsb_w = 280;
  int curr_y = dsb_y + 180;

  if (civ_input_is_mouse_over(input, dsb_x + 10, curr_y, dsb_w - 20, 40)) {
    civ_diplomacy_system_propose_treaty(game->diplomacy_system, "player",
                                        "rival_kingdom",
                                        CIV_TREATY_TYPE_TRADE_AGREEMENT, 30);
    return true;
  }
  curr_y += 50;
  if (civ_input_is_mouse_over(input, dsb_x + 10, curr_y, dsb_w - 20, 40)) {
    civ_diplomacy_system_propose_treaty(game->diplomacy_system, "player",
                                        "rival_kingdom",
                                        CIV_TREATY_TYPE_NON_AGGRESSION, 30);
    return true;
  }
  return false;
}
