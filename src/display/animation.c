#include "display/animation.h"
#include <math.h>
#include <string.h>

static float ease_out_cubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }
static float ease_in_out_quad(float t) {
  return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

void civ_tween_init(civ_tween_t *anim) { memset(anim, 0, sizeof(*anim)); }

void civ_tween_start(civ_tween_t *anim, civ_tween_type_t type, float duration,
                    float from, float to) {
  anim->type = type;
  anim->duration = duration;
  anim->elapsed = 0.0f;
  anim->from = from;
  anim->to = to;
  anim->active = true;
  anim->done = false;
}

void civ_tween_update(civ_tween_t *anim, float dt) {
  if (!anim->active) return;
  anim->elapsed += dt;
  if (anim->elapsed >= anim->duration) {
    anim->elapsed = anim->duration;
    anim->done = true;
    anim->active = (anim->type == CIV_TWEEN_FADE_IN ||
                    anim->type == CIV_TWEEN_SLIDE_UP ||
                    anim->type == CIV_TWEEN_SLIDE_DOWN ||
                    anim->type == CIV_TWEEN_SLIDE_LEFT ||
                    anim->type == CIV_TWEEN_SLIDE_RIGHT ||
                    anim->type == CIV_TWEEN_SCALE_IN);
  }
}

float civ_tween_value(const civ_tween_t *anim) {
  if (!anim->active && !anim->done) return anim->from;
  float t = (anim->duration > 0.0f) ? anim->elapsed / anim->duration : 1.0f;
  float e =
      (anim->type == CIV_TWEEN_FADE_OUT || anim->type == CIV_TWEEN_SCALE_OUT)
          ? ease_in_out_quad(t)
          : ease_out_cubic(t);
  return anim->from + (anim->to - anim->from) * e;
}

bool civ_tween_is_active(const civ_tween_t *anim) {
  return anim->active || !anim->done;
}

void civ_panel_anim_init(civ_panel_anim_t *pa) {
  memset(pa, 0, sizeof(*pa));
  pa->visible = false;
}

void civ_panel_anim_show(civ_panel_anim_t *pa, float duration) {
  pa->visible = true;
  civ_tween_start(&pa->fade, CIV_TWEEN_FADE_IN, duration, 0.0f, 1.0f);
  civ_tween_start(&pa->slide_y, CIV_TWEEN_SLIDE_DOWN, duration, -20.0f, 0.0f);
  civ_tween_start(&pa->scale, CIV_TWEEN_SCALE_IN, duration, 0.95f, 1.0f);
}

void civ_panel_anim_hide(civ_panel_anim_t *pa, float duration) {
  pa->visible = false;
  civ_tween_start(&pa->fade, CIV_TWEEN_FADE_OUT, duration, 1.0f, 0.0f);
  civ_tween_start(&pa->slide_y, CIV_TWEEN_SLIDE_UP, duration, 0.0f, -20.0f);
  civ_tween_start(&pa->scale, CIV_TWEEN_SCALE_OUT, duration, 1.0f, 0.95f);
}

void civ_panel_anim_update(civ_panel_anim_t *pa, float dt) {
  civ_tween_update(&pa->fade, dt);
  civ_tween_update(&pa->slide_x, dt);
  civ_tween_update(&pa->slide_y, dt);
  civ_tween_update(&pa->scale, dt);
}

float civ_panel_anim_alpha(const civ_panel_anim_t *pa) {
  return civ_tween_value(&pa->fade);
}
float civ_panel_anim_offset_x(const civ_panel_anim_t *pa) {
  return civ_tween_value(&pa->slide_x);
}
float civ_panel_anim_offset_y(const civ_panel_anim_t *pa) {
  return civ_tween_value(&pa->slide_y);
}
float civ_panel_anim_scale(const civ_panel_anim_t *pa) {
  return civ_tween_value(&pa->scale);
}
bool civ_panel_anim_is_visible(const civ_panel_anim_t *pa) {
  return pa->visible || civ_tween_is_active(&pa->fade);
}
