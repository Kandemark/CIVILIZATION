#include "../../../include/ui/widget/progress_bar.h"
#include "../../../include/display/theme.h"
#include "../../../include/engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_progress_bar_t *civ_progress_bar_create(const char *id, float x, float y,
                                            float w, float h) {
  civ_progress_bar_t *pb = (civ_progress_bar_t *)malloc(sizeof(civ_progress_bar_t));
  if (!pb) return NULL;
  memset(pb, 0, sizeof(*pb));
  civ_widget_init(&pb->base, CIV_WIDGET_PROGRESS_BAR, id, x, y, w, h);
  pb->value = 0.0f;
  pb->color_fg = g_theme.primary;
  pb->color_bg = g_theme.bg_medium;
  pb->vertical = false;
  return pb;
}

void civ_progress_bar_destroy(civ_progress_bar_t *pb) { free(pb); }

void civ_progress_bar_set_value(civ_progress_bar_t *pb, float value) {
  if (!pb) return;
  if (value < 0.0f) value = 0.0f;
  if (value > 1.0f) value = 1.0f;
  pb->value = value;
}

void civ_progress_bar_render(civ_progress_bar_t *pb, SDL_Renderer *r) {
  if (!pb || !pb->base.visible || !r) return;

  int x = (int)pb->base.x, y = (int)pb->base.y;
  int w = (int)pb->base.w, h = (int)pb->base.h;

  /* Background track */
  civ_render_rect_filled(r, x, y, w, h, pb->color_bg);
  civ_render_rect_outline(r, x, y, w, h, 0x1A2A3A, 1);

  /* Fill */
  if (pb->vertical) {
    int fill_h = (int)((float)h * pb->value);
    civ_render_rect_filled(r, x, y + h - fill_h, w, fill_h, pb->color_fg);
  } else {
    int fill_w = (int)((float)w * pb->value);
    civ_render_rect_filled(r, x, y, fill_w, h, pb->color_fg);
  }
}
