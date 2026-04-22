#include "ui/widget/toggle.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_toggle_t *civ_toggle_create(const char *id, float x, float y,
                                float w, float h, const char *label) {
  civ_toggle_t *t = calloc(1, sizeof(*t));
  if (!t) return NULL;
  civ_widget_init(&t->base, CIV_WIDGET_TOGGLE, id, x, y, w, h);
  if (label) strncpy(t->label, label, sizeof(t->label) - 1);
  t->value = false;
  t->color_on = g_theme.primary;
  t->color_off = g_theme.bg_medium;
  t->color_thumb = 0xFFFFFF;
  t->base.state = CIV_WIDGET_NORMAL;
  return t;
}

void civ_toggle_destroy(civ_toggle_t *t) { free(t); }

void civ_toggle_set_value(civ_toggle_t *t, bool value) {
  if (!t) return;
  t->value = value;
  civ_tween_start(&t->thumb_tween, CIV_TWEEN_SLIDE_RIGHT, 0.15f,
                  value ? 0.0f : 1.0f, value ? 1.0f : 0.0f);
}

bool civ_toggle_get_value(const civ_toggle_t *t) {
  return t ? t->value : false;
}

void civ_toggle_update(civ_toggle_t *t, civ_input_state_t *input, float dt) {
  if (!t || !input) return;
  t->base.state = civ_widget_compute_state(&t->base, input, true);
  if (t->base.state == CIV_WIDGET_PRESSED) {
    civ_toggle_set_value(t, !t->value);
  }
  civ_tween_update(&t->thumb_tween, dt);
}

void civ_toggle_render(civ_toggle_t *t, SDL_Renderer *r) {
  if (!t || !r || !t->base.visible) return;

  float x = t->base.x, y = t->base.y, w = t->base.w, h = t->base.h;
  float thumb_w = h - 4.0f;

  /* Track background with slight animation */
  uint32_t bg = t->value ? t->color_on : t->color_off;
  uint8_t alpha = (t->base.state == CIV_WIDGET_DISABLED) ? 80 : 200;
  civ_render_rect_filled_alpha(r, (int)x, (int)y, (int)w, (int)h, bg, alpha);

  /* Thumb position */
  float t_val = civ_tween_value(&t->thumb_tween);
  if (!t->thumb_tween.active) t_val = t->value ? 1.0f : 0.0f;
  float thumb_x = x + 2.0f + (w - thumb_w - 4.0f) * t_val;
  civ_render_rect_filled(r, (int)thumb_x, (int)(y + 2.0f),
                         (int)thumb_w, (int)(h - 4.0f), t->color_thumb);
}
