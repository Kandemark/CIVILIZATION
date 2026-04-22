#include "ui/panel/unit_sidebar.h"
#include "engine/renderer.h"
#include <stdio.h>
#include <string.h>

void civ_unit_sidebar_render(SDL_Renderer *r, civ_unit_t *unit, civ_font_t *font,
                             civ_input_state_t *input) {
  if (!r || !unit || !font) return;

  int sb_w = 280, sb_h = 320, sb_x = 20, sb_y = 60;

  civ_render_rect_filled_alpha(r, sb_x, sb_y, sb_w, sb_h, 0x050A14, 230);
  civ_render_rect_outline(r, sb_x, sb_y, sb_w, sb_h, 0x00A0FF, 1);
  civ_render_line(r, sb_x, sb_y + 40, sb_x + sb_w, sb_y + 40, 0x1A2A3A);

  char buf[128];
  sprintf(buf, "%s", unit->name);
  civ_font_render_aligned(r, font, buf, sb_x + 15, sb_y, sb_w - 30, 40,
                          0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  int curr_y = sb_y + 55;

  sprintf(buf, "TYPE: %s",
          unit->unit_type == CIV_UNIT_TYPE_INFANTRY   ? "Infantry"
          : unit->unit_type == CIV_UNIT_TYPE_SETTLER ? "Settler"
                                                      : "Military");
  civ_font_render_aligned(r, font, buf, sb_x + 15, curr_y, sb_w - 30, 25,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 30;

  sprintf(buf, "STRENGTH: %d / %d", unit->current_strength, unit->max_strength);
  civ_font_render_aligned(r, font, buf, sb_x + 15, curr_y, sb_w - 30, 25,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 30;

  sprintf(buf, "MORALE: %.0f%%", unit->morale * 100.0f);
  civ_font_render_aligned(r, font, buf, sb_x + 15, curr_y, sb_w - 30, 25,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 30;

  sprintf(buf, "POS: (%d, %d)", unit->x, unit->y);
  civ_font_render_aligned(r, font, buf, sb_x + 15, curr_y, sb_w - 30, 25,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 40;

  if (unit->has_moved) {
    civ_font_render_aligned(r, font, "MOVEMENT EXHAUSTED", sb_x + 15, curr_y,
                            sb_w - 30, 25, 0xFF4444, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);
  } else {
    civ_font_render_aligned(r, font, "READY TO MOVE", sb_x + 15, curr_y,
                            sb_w - 30, 25, 0x44FF44, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);
  }
  curr_y += 35;

  if (unit->unit_type == CIV_UNIT_TYPE_SETTLER && !unit->has_moved) {
    int f_btn_x = sb_x + 15, f_btn_y = sb_y + 250, f_btn_w = sb_w - 30,
        f_btn_h = 40;
    bool f_hov = (input->mouse_x >= f_btn_x && input->mouse_x <= f_btn_x + f_btn_w &&
                  input->mouse_y >= f_btn_y && input->mouse_y <= f_btn_y + f_btn_h);
    civ_render_rect_filled(r, f_btn_x, f_btn_y, f_btn_w, f_btn_h,
                           f_hov ? 0x00A0FF : 0x005A99);
    civ_font_render_aligned(r, font, "FOUND CITY", f_btn_x, f_btn_y, f_btn_w,
                            f_btn_h, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  }
}

int civ_unit_sidebar_click(civ_unit_t *unit, civ_input_state_t *input) {
  if (!unit || !input || unit->unit_type != CIV_UNIT_TYPE_SETTLER ||
      unit->has_moved)
    return 0;
  if (input->mouse_x >= 35 && input->mouse_x <= 285 && input->mouse_y >= 310 &&
      input->mouse_y <= 350)
    return 1; /* Found City */
  return 0;
}
