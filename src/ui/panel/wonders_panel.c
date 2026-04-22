#include "ui/panel/wonders_panel.h"
#include "core/world/wonders.h"
#include "engine/renderer.h"
#include <stdio.h>

void civ_wonders_panel_render(SDL_Renderer *r, civ_game_t *game,
                              civ_font_t *font, civ_input_state_t *input,
                              int win_w, int win_h) {
  if (!r || !game || !game->wonder_manager || !font) return;

  civ_wonder_manager_t *mgr = game->wonder_manager;
  int rpb_w = 650, rpb_h = 550;
  int rpb_x = (win_w - rpb_w) / 2, rpb_y = (win_h - rpb_h) / 2;

  civ_render_rect_filled_alpha(r, rpb_x, rpb_y, rpb_w, rpb_h, 0x111111, 230);
  civ_render_rect_outline(r, rpb_x, rpb_y, rpb_w, rpb_h, 0xFFFF00, 2);

  civ_font_render_aligned(r, font, "WONDERS OF THE WORLD", win_w / 2, rpb_y + 30,
                          400, 40, 0x00FFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  int curr_y = rpb_y + 80, box_h = 70;
  for (int i = 0; i < CIV_WONDER_COUNT; i++) {
    civ_wonder_t *w = &mgr->wonders[i];
    uint32_t box_color = w->is_built ? 0x444444 : 0x222222;
    civ_render_rect_filled(r, rpb_x + 20, curr_y, rpb_w - 40, box_h, box_color);
    civ_render_rect_outline(r, rpb_x + 20, curr_y, rpb_w - 40, box_h, 0x666666,
                            1);

    uint32_t text_color = w->is_built ? 0x888888 : 0xFFFFFF;
    civ_font_render_aligned(r, font, w->name, rpb_x + 30, curr_y + 10, 300, 30,
                            text_color, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    civ_font_render_aligned(r, font, w->description, rpb_x + 30, curr_y + 40,
                            400, 20, 0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

    if (w->is_built) {
      char buf[64];
      snprintf(buf, sizeof(buf), "Built by: %s", w->builder_id);
      civ_font_render_aligned(r, font, buf, rpb_x + rpb_w - 180, curr_y + 25,
                              150, 25, 0xAA8800, CIV_ALIGN_RIGHT,
                              CIV_VALIGN_MIDDLE);
    } else {
      char buf[64] = "";
      if (w->effects.science_mult > 0)
        snprintf(buf, 64, "+%d%% Science", (int)(w->effects.science_mult * 100));
      else if (w->effects.production_mult > 0)
        snprintf(buf, 64, "+%d%% Production",
                 (int)(w->effects.production_mult * 100));
      else if (w->effects.gold_mult > 0)
        snprintf(buf, 64, "+%d%% Gold", (int)(w->effects.gold_mult * 100));
      civ_font_render_aligned(r, font, buf, rpb_x + rpb_w - 180, curr_y + 25,
                              150, 25, 0x00FF00, CIV_ALIGN_RIGHT,
                              CIV_VALIGN_MIDDLE);
    }
    curr_y += box_h + 10;
  }
  (void)input;
}
