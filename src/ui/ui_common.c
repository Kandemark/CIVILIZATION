/**
 * @file ui_common.c
 * @brief Common UI utilities implementation
 */

#include "../../include/ui/ui_common.h"
#include <math.h>

void civ_anim_update(civ_anim_float_t *anim) {
  if (!anim)
    return;

  anim->current += (anim->target - anim->current) * anim->speed;

  /* Snap to target if very close */
  if (fabsf(anim->target - anim->current) < 0.0001f) {
    anim->current = anim->target;
  }
}

void civ_anim_set_target(civ_anim_float_t *anim, float target) {
  if (!anim)
    return;
  anim->target = target;
}

void civ_anim_init(civ_anim_float_t *anim, float initial, float speed) {
  if (!anim)
    return;
  anim->current = initial;
  anim->target = initial;
  anim->speed = speed;
}

bool civ_rect_contains(const civ_rect_t *rect, int x, int y) {
  if (!rect)
    return false;
  return (x >= rect->x && x <= rect->x + rect->w && y >= rect->y &&
          y <= rect->y + rect->h);
}

float civ_clampf(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

int civ_clampi(int value, int min, int max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

float civ_lerpf(float a, float b, float t) { return a + (b - a) * t; }
