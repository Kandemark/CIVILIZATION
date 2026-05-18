#include "../../../include/ui/panel/settlement_sidebar.h"
#include "../../../include/engine/renderer.h"
#include <stdio.h>

void civ_settlement_sidebar_render(SDL_Renderer *r, civ_settlement_t *sett,
                                   civ_font_t *font, civ_input_state_t *input) {
  if (!r || !sett || !font) return;

  int ssb_w = 280, ssb_h = 320, ssb_x = 20, ssb_y = 60;

  civ_render_rect_filled_alpha(r, ssb_x, ssb_y, ssb_w, ssb_h, 0x0A0F1E, 240);
  civ_render_rect_outline(r, ssb_x, ssb_y, ssb_w, ssb_h, 0xFFFF00, 1);
  civ_render_line(r, ssb_x, ssb_y + 40, ssb_x + ssb_w, ssb_y + 40, 0x1A2A3A);

  civ_font_render_aligned(r, font, sett->name, ssb_x + 15, ssb_y, ssb_w - 30,
                          40, 0xFFFF00, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  char buf[128];
  int curr_y = ssb_y + 55;

  const char *tier_names[] = {"Hamlet", "Village",    "Town",
                              "City",   "Metropolis", "Capital"};
  sprintf(buf, "TIER: %s", tier_names[sett->tier]);
  civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 30,
                          0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 25;

  sprintf(buf, "POPULATION: %lld", sett->population);
  civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 30,
                          0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 25;

  sprintf(buf, "ATTRACTIVENESS: %.2f", sett->attractiveness);
  civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 30,
                          0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 25;

  /* Loyalty bar */
  civ_font_render_aligned(r, font, "LOYALTY", ssb_x + 15, curr_y, ssb_w - 30,
                          15, 0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  uint32_t loyalty_color = sett->loyalty > 0.4f ? 0x00FF88 : 0xFF3300;
  civ_render_rect_filled(r, ssb_x + 15, curr_y + 18,
                         (int)((ssb_w - 30) * sett->loyalty), 6, loyalty_color);
  civ_render_rect_outline(r, ssb_x + 15, curr_y + 18, ssb_w - 30, 6, 0x444444,
                          1);
  curr_y += 35;

  /* Unrest bar */
  civ_font_render_aligned(r, font, "UNREST", ssb_x + 15, curr_y, ssb_w - 30, 15,
                          0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(r, ssb_x + 15, curr_y + 18,
                         (int)((ssb_w - 30) * sett->unrest), 6, 0xFF6600);
  civ_render_rect_outline(r, ssb_x + 15, curr_y + 18, ssb_w - 30, 6, 0x444444,
                          1);
  curr_y += 35;

  /* Culture stats */
  sprintf(buf, "CULTURE: %.1f (+%.1f/YR)", sett->accumulated_culture,
          sett->culture_yield);
  civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 30,
                          0x00FFCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 25;

  float next_threshold = 0;
  if (sett->territory_radius == 2) next_threshold = 20.0f;
  else if (sett->territory_radius == 3) next_threshold = 100.0f;
  else if (sett->territory_radius == 4) next_threshold = 500.0f;

  if (next_threshold > 0) {
    sprintf(buf, "EXPANSION: %.0f%%",
            (sett->accumulated_culture / next_threshold) * 100.0f);
    civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 30,
                            0x00FFCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }
  curr_y += 35;

  /* Recruitment section */
  civ_render_line(r, ssb_x + 10, curr_y, ssb_x + ssb_w - 10, curr_y, 0x336644);
  curr_y += 10;
  civ_font_render_aligned(r, font, "RECRUIT UNITS", ssb_x + 15, curr_y,
                          ssb_w - 30, 20, 0x00FF88, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 25;

  if (sett->is_producing) {
    float perc = sett->production_progress / sett->production_target;
    civ_render_rect_filled(r, ssb_x + 15, curr_y, ssb_w - 30, 25, 0x1A2A1A);
    civ_render_rect_filled(r, ssb_x + 15, curr_y, (int)((ssb_w - 30) * perc), 25,
                           0x00A0FF);
    sprintf(buf, "TRAINING... %d%%", (int)(perc * 100));
    civ_font_render_aligned(r, font, buf, ssb_x + 15, curr_y, ssb_w - 30, 25,
                            0xFFFFFF, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  } else {
    /* Infantry button */
    bool h_inf = (input->mouse_x >= ssb_x + 10 && input->mouse_x <= ssb_x + ssb_w - 10 &&
                  input->mouse_y >= curr_y && input->mouse_y <= curr_y + 40);
    civ_render_rect_filled(r, ssb_x + 10, curr_y, ssb_w - 20, 40,
                           h_inf ? 0x2A3A2A : 0x1A2A1A);
    civ_font_render_aligned(r, font, "RECRUIT INFANTRY (30)", ssb_x + 15, curr_y,
                            ssb_w - 30, 40, 0xFFFFFF, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);
    curr_y += 50;

    /* Settler button */
    if (sett->population > 500) {
      bool h_set = (input->mouse_x >= ssb_x + 10 &&
                    input->mouse_x <= ssb_x + ssb_w - 10 &&
                    input->mouse_y >= curr_y && input->mouse_y <= curr_y + 40);
      civ_render_rect_filled(r, ssb_x + 10, curr_y, ssb_w - 20, 40,
                             h_set ? 0x2A3A2A : 0x1A2A1A);
      civ_font_render_aligned(r, font, "RECRUIT SETTLER (80)", ssb_x + 15,
                              curr_y, ssb_w - 30, 40, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_MIDDLE);
    } else {
      civ_font_render_aligned(r, font, "SETTLER (Needs 500 Pop)", ssb_x + 15,
                              curr_y, ssb_w - 30, 40, 0x666666, CIV_ALIGN_LEFT,
                              CIV_VALIGN_MIDDLE);
    }
  }
}

int civ_settlement_sidebar_click(civ_settlement_t *sett,
                                 civ_input_state_t *input) {
  if (!sett || !input || sett->is_producing) return 0;

  int ssb_x = 20, ssb_y = 60;
  int r1_y = ssb_y + 245, r2_y = r1_y + 50, r_w = 260, r_h = 40;

  if (input->mouse_x >= ssb_x + 10 && input->mouse_x <= ssb_x + 10 + r_w &&
      input->mouse_y >= r1_y && input->mouse_y <= r1_y + r_h)
    return 1; /* Infantry */

  if (sett->population > 500 &&
      input->mouse_x >= ssb_x + 10 && input->mouse_x <= ssb_x + 10 + r_w &&
      input->mouse_y >= r2_y && input->mouse_y <= r2_y + r_h)
    return 2; /* Settler */

  return 0;
}
