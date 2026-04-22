#include "ui/widget/button.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_widget_button_t *civ_widget_button_create(const char *id, float x, float y,
                                              float w, float h,
                                              const char *text) {
  civ_widget_button_t *btn =
      (civ_widget_button_t *)malloc(sizeof(civ_widget_button_t));
  if (!btn) return NULL;
  memset(btn, 0, sizeof(*btn));
  civ_widget_init(&btn->base, CIV_WIDGET_BUTTON, id, x, y, w, h);
  if (text) strncpy(btn->text, text, sizeof(btn->text) - 1);
  btn->color_bg = g_theme.bg_medium;
  btn->color_hover = g_theme.bg_light;
  btn->color_press = g_theme.primary_dark;
  btn->color_text = g_theme.text_primary;
  btn->color_border = 0x1A2A3A;
  btn->corner_radius = 6;
  civ_tween_init(&btn->hover_tween);
  return btn;
}

void civ_widget_button_destroy(civ_widget_button_t *btn) { free(btn); }

void civ_widget_button_set_text(civ_widget_button_t *btn, const char *text) {
  if (!btn) return;
  strncpy(btn->text, text ? text : "", sizeof(btn->text) - 1);
}

void civ_widget_button_set_enabled(civ_widget_button_t *btn, bool enabled) {
  if (!btn) return;
  btn->base.enabled = enabled;
}

void civ_widget_button_set_position(civ_widget_button_t *btn, float x, float y) {
  if (!btn) return;
  btn->base.x = x;
  btn->base.y = y;
}

void civ_widget_button_update(civ_widget_button_t *btn, civ_input_state_t *input,
                              float dt) {
  if (!btn || !btn->base.visible || !btn->base.enabled) return;

  btn->base.state =
      civ_widget_compute_state(&btn->base, input, true);

  float target = (btn->base.state >= CIV_WIDGET_HOVERED) ? 1.0f : 0.0f;
  if (!btn->hover_tween.active && btn->hover_tween.done)
    civ_tween_start(&btn->hover_tween, CIV_TWEEN_FADE_IN, 0.15f,
                    btn->hover_tween.to, target);
  else if (fabsf(btn->hover_tween.to - target) > 0.01f)
    civ_tween_start(&btn->hover_tween, CIV_TWEEN_FADE_IN, 0.15f,
                    btn->hover_tween.to, target);
  civ_tween_update(&btn->hover_tween, dt);

  btn->clicked = (btn->base.state == CIV_WIDGET_PRESSED &&
                  input->mouse_left_pressed);
}

void civ_widget_button_render(civ_widget_button_t *btn, SDL_Renderer *r,
                              civ_font_t *font) {
  if (!btn || !btn->base.visible || !r) return;

  int x = (int)btn->base.x, y = (int)btn->base.y;
  int w = (int)btn->base.w, h = (int)btn->base.h;

  float hover_amt = civ_tween_value(&btn->hover_tween);

  /* Background */
  uint32_t bg = btn->color_bg;
  if (btn->base.state == CIV_WIDGET_HOVERED)
    bg = civ_theme_mix(btn->color_bg, btn->color_hover, hover_amt);
  if (btn->base.state == CIV_WIDGET_PRESSED) bg = btn->color_press;
  if (!btn->base.enabled) bg = 0x111111;

  civ_render_rect_filled_alpha(r, x, y, w, h, bg, 240);

  /* Border */
  uint32_t border = (btn->base.state == CIV_WIDGET_HOVERED || hover_amt > 0.01f)
                        ? civ_theme_mix(btn->color_border, g_theme.primary,
                                        hover_amt)
                        : btn->color_border;
  civ_render_rect_outline(r, x, y, w, h, border, 1);

  /* Text */
  if (font && btn->text[0]) {
    uint32_t tc = btn->base.enabled ? btn->color_text : 0x4A5A6A;
    civ_font_render_aligned(r, font, btn->text, x, y, w, h, tc,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }
}

bool civ_widget_button_was_clicked(civ_widget_button_t *btn) {
  return btn ? btn->clicked : false;
}
