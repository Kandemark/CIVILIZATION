#include "ui/widget/slider.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_slider_t *civ_slider_create(const char *id, float x, float y,
                                float w, float h, float min, float max) {
  civ_slider_t *s = calloc(1, sizeof(*s));
  if (!s) return NULL;
  civ_widget_init(&s->base, CIV_WIDGET_SLIDER, id, x, y, w, h);
  s->min = min; s->max = max;
  s->value = min;
  s->color_track = g_theme.bg_medium;
  s->color_fill = g_theme.primary;
  s->color_thumb = 0xCCCCCC;
  return s;
}

void civ_slider_destroy(civ_slider_t *s) { free(s); }

void civ_slider_set_value(civ_slider_t *s, float value) {
  if (!s) return;
  s->value = value;
  if (s->value < s->min) s->value = s->min;
  if (s->value > s->max) s->value = s->max;
}

float civ_slider_get_value(const civ_slider_t *s) {
  return s ? s->value : 0.0f;
}

void civ_slider_update(civ_slider_t *s, civ_input_state_t *input, float dt) {
  if (!s || !input) return;
  (void)dt;

  bool hover = civ_widget_is_hovered(&s->base, input);

  if (hover && input->mouse_left_pressed) {
    s->dragging = true;
  }
  if (s->dragging) {
    if (input->mouse_left_down) {
      float ratio;
      if (s->vertical) {
        ratio = 1.0f - ((float)input->mouse_y - s->base.y) / s->base.h;
      } else {
        ratio = ((float)input->mouse_x - s->base.x) / s->base.w;
      }
      if (ratio < 0.0f) ratio = 0.0f;
      if (ratio > 1.0f) ratio = 1.0f;
      s->value = s->min + (s->max - s->min) * ratio;
    } else {
      s->dragging = false;
    }
  }
}

void civ_slider_render(civ_slider_t *s, SDL_Renderer *r) {
  if (!s || !r || !s->base.visible) return;
  int x = (int)s->base.x, y = (int)s->base.y;
  int w = (int)s->base.w, h = (int)s->base.h;

  float ratio = (s->value - s->min) / (s->max - s->min);
  if (ratio < 0.0f) ratio = 0.0f;
  if (ratio > 1.0f) ratio = 1.0f;

  if (s->vertical) {
    civ_render_rect_filled_alpha(r, x + w/2 - 2, y, 4, h, s->color_track, 180);
    int fill_h = (int)((float)h * ratio);
    civ_render_rect_filled(r, x + w/2 - 2, y + h - fill_h, 4, fill_h, s->color_fill);
    int thumb_y = y + h - fill_h - 5;
    civ_render_rect_filled(r, x, thumb_y, w, 10, s->color_thumb);
  } else {
    civ_render_rect_filled_alpha(r, x, y + h/2 - 2, w, 4, s->color_track, 180);
    int fill_w = (int)((float)w * ratio);
    civ_render_rect_filled(r, x, y + h/2 - 2, fill_w, 4, s->color_fill);
    int thumb_x = x + fill_w - 5;
    civ_render_rect_filled(r, thumb_x, y, 10, h, s->color_thumb);
  }
}
