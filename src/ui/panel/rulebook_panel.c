#include "ui/panel/rulebook_panel.h"
#include "core/governance/government.h"
#include "core/governance/territorial/subdivision.h"
#include "core/governance/institutions/institution.h"
#include "engine/renderer.h"
#include <stdio.h>

void civ_rulebook_panel_render(SDL_Renderer *r, civ_game_t *game,
                               civ_font_t *font, civ_input_state_t *input,
                               int win_w, int win_h) {
  if (!r || !game || !game->government || !font) return;

  civ_government_t *gov = game->government;
  int rbe_w = 700, rbe_h = 550;
  int rbe_x = (win_w - rbe_w) / 2, rbe_y = (win_h - rbe_h) / 2;

  civ_render_rect_filled_alpha(r, rbe_x, rbe_y, rbe_w, rbe_h, 0x050A0F, 245);
  civ_render_rect_outline(r, rbe_x, rbe_y, rbe_w, rbe_h, 0x00A0FF, 2);

  civ_font_render_aligned(r, font, "STATE RULEBOOK & INSTITUTIONS", rbe_x,
                          rbe_y + 20, rbe_w, 40, 0xFFFFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_TOP);

  int curr_y = rbe_y + 80;

  /* Legislative Logic */
  civ_font_render_aligned(r, font, "LEGISLATIVE LOGIC", rbe_x + 30, curr_y, 300,
                          30, 0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  char buf[128];
  sprintf(buf, "Voting Threshold: %.0f%% (Simple Majority)",
          gov->legislative_threshold * 100.0f);
  civ_font_render_aligned(r, font, buf, rbe_x + 40, curr_y + 30, 300, 20,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 70;

  /* Institutions */
  civ_font_render_aligned(r, font, "ACTIVE INSTITUTIONS", rbe_x + 30, curr_y,
                          300, 30, 0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 35;

  if (gov->institution_manager) {
    for (size_t i = 0; i < gov->institution_manager->count; i++) {
      civ_institution_t *inst = &gov->institution_manager->items[i];
      civ_render_rect_filled(r, rbe_x + 40, curr_y, rbe_w - 80, 50, 0x1A2A3A);
      civ_font_render_aligned(r, font, inst->name, rbe_x + 50, curr_y + 5, 300,
                              20, 0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      sprintf(buf, "Stature: %.1f | Maint: %.1f Gold", inst->stature,
              inst->maintenance_cost);
      civ_font_render_aligned(r, font, buf, rbe_x + 50, curr_y + 25, 400, 20,
                              0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      curr_y += 60;
    }
  }

  /* Subdivisions */
  civ_font_render_aligned(r, font, "ADMINISTRATIVE SUBDIVISIONS", rbe_x + 30,
                          curr_y, 300, 30, 0x00A0FF, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 35;
  if (gov->subdivision_manager) {
    for (size_t i = 0; i < gov->subdivision_manager->count; i++) {
      civ_subdivision_t *sub = &gov->subdivision_manager->items[i];
      sprintf(buf, "%s (%s) - Autonomy: %.0f%%", sub->name,
              (sub->type == CIV_SUBDIVISION_COLONY ? "COLONY" : "STATE"),
              sub->autonomy * 100.0f);
      civ_font_render_aligned(r, font, buf, rbe_x + 40, curr_y, 500, 20,
                              0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      curr_y += 25;
    }
  }
  (void)input;
}
