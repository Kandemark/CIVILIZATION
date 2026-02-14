/**
 * @file button.c
 * @brief UI button widget implementation
 */

#include "../../include/ui/button.h"
#include "../../include/engine/renderer.h"
#include "../../include/ui/ui_common.h"

civ_button_t civ_button_create(int id, int x, int y, int w, int h,
                               const char *label) {
  civ_button_t btn;
  btn.id = id;
  btn.x = x;
  btn.y = y;
  btn.w = w;
  btn.h = h;
  btn.label = label;
  btn.highlighted = false;
  btn.enabled = true;
  btn.color = CIV_COLOR_BG_LIGHT;
  btn.text_color = CIV_COLOR_TEXT;
  btn.hover_anim = 0.0f;
  btn.active_anim = 0.0f;
  return btn;
}

bool civ_button_render(SDL_Renderer *renderer, civ_font_t *font,
                       civ_button_t *button, civ_input_state_t *input) {
  if (!renderer || !font || !button || !input)
    return false;

  bool hovered =
      button->enabled && civ_input_is_mouse_over(input, button->x, button->y,
                                                 button->w, button->h);

  if (hovered) {
    input->hot_id = button->id;
  }

  /* Smooth animation updates (approx 60fps delta) */
  float target_hover = hovered ? 1.0f : 0.0f;
  button->hover_anim += (target_hover - button->hover_anim) * 0.15f;

  float target_active = (input->active_id == button->id) ? 1.0f : 0.0f;
  button->active_anim += (target_active - button->active_anim) * 0.30f;

  bool clicked = false;
  if (input->last_clicked_id == button->id) {
    clicked = true;
  }

  /* 1. Base Shadow/Glow */
  if (button->hover_anim > 0.01f) {
    civ_render_rounded_rect(renderer, button->x - 2, button->y - 2,
                            button->w + 4, button->h + 4, 12, CIV_COLOR_GLOW);
  }

  /* 2. Button Body */
  uint8_t r1 = (CIV_COLOR_BG_LIGHT >> 16) & 0xFF;
  uint8_t g1 = (CIV_COLOR_BG_LIGHT >> 8) & 0xFF;
  uint8_t b1 = CIV_COLOR_BG_LIGHT & 0xFF;

  uint8_t r2 = (CIV_COLOR_PRIMARY_DARK >> 16) & 0xFF;
  uint8_t g2 = (CIV_COLOR_PRIMARY_DARK >> 8) & 0xFF;
  uint8_t b2 = CIV_COLOR_PRIMARY_DARK & 0xFF;

  uint8_t r = (uint8_t)civ_lerpf((float)r1, (float)r2, button->hover_anim);
  uint8_t g = (uint8_t)civ_lerpf((float)g1, (float)g2, button->hover_anim);
  uint8_t b = (uint8_t)civ_lerpf((float)b1, (float)b2, button->hover_anim);

  if (!button->enabled) {
    r = g = b = 0x22;
  }

  uint32_t current_bg = (r << 16) | (g << 8) | b;
  civ_render_rounded_rect(renderer, button->x, button->y, button->w, button->h,
                          10, current_bg);

  /* 3. Border Glow on Hover */
  uint32_t border_color = button->highlighted ? CIV_COLOR_ACCENT : 0x3A4A5A;
  if (hovered || button->hover_anim > 0.1f) {
    border_color = CIV_COLOR_PRIMARY;
  }
  civ_render_rect_outline(renderer, button->x, button->y, button->w, button->h,
                          border_color, 1);

  /* 4. Text Label */
  uint32_t text_color =
      button->enabled ? button->text_color : CIV_COLOR_TEXT_DARK;
  if (hovered)
    text_color = CIV_COLOR_TEXT;

  civ_font_render_aligned(renderer, font, button->label, button->x, button->y,
                          button->w, button->h, text_color, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  return clicked;
}

void civ_button_set_position(civ_button_t *button, int x, int y) {
  if (!button)
    return;
  button->x = x;
  button->y = y;
}

void civ_button_set_size(civ_button_t *button, int w, int h) {
  if (!button)
    return;
  button->w = w;
  button->h = h;
}

void civ_button_set_enabled(civ_button_t *button, bool enabled) {
  if (!button)
    return;
  button->enabled = enabled;
}

void civ_button_set_highlighted(civ_button_t *button, bool highlighted) {
  if (!button)
    return;
  button->highlighted = highlighted;
}

bool civ_button_is_hovered(const civ_button_t *button,
                           const civ_input_state_t *input) {
  if (!button || !input)
    return false;
  return civ_input_is_mouse_over(input, button->x, button->y, button->w,
                                 button->h);
}
