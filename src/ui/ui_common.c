#include "../../include/ui/ui_common.h"

bool civ_rect_contains(const civ_rect_t *rect, int x, int y) {
  if (!rect) return false;
  return (x >= rect->x && x <= rect->x + rect->w && y >= rect->y &&
          y <= rect->y + rect->h);
}

float civ_clampf(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

int civ_clampi(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

float civ_lerpf(float a, float b, float t) { return a + (b - a) * t; }
