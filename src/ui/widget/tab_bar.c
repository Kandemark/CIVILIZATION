#include "ui/widget/tab_bar.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_tab_bar_t *civ_tab_bar_create(const char *id, float x, float y,
                                  float w, float h) {
  civ_tab_bar_t *tb = calloc(1, sizeof(*tb));
  if (!tb) return NULL;
  civ_widget_init(&tb->base, CIV_WIDGET_TAB_BAR, id, x, y, w, h);
  tb->active_tab = -1;
  tb->color_bg = g_theme.bg_dark;
  tb->color_active = g_theme.primary;
  tb->color_hover = g_theme.bg_medium;
  tb->color_text = g_theme.text_primary;
  return tb;
}

void civ_tab_bar_destroy(civ_tab_bar_t *tb) { free(tb); }

void civ_tab_bar_add_tab(civ_tab_bar_t *tb, const char *label) {
  if (!tb || !label || tb->tab_count >= CIV_TAB_BAR_MAX_TABS) return;
  strncpy(tb->labels[tb->tab_count], label, CIV_TAB_LABEL_LEN - 1);
  tb->tab_count++;
}

void civ_tab_bar_set_active(civ_tab_bar_t *tb, int index) {
  if (!tb || index < 0 || index >= tb->tab_count) return;
  tb->active_tab = index;
}

int civ_tab_bar_get_active(const civ_tab_bar_t *tb) {
  return tb ? tb->active_tab : -1;
}

static bool pt_in(float px, float py, float rx, float ry, float rw, float rh) {
  return px >= rx && py >= ry && px <= rx + rw && py <= ry + rh;
}

void civ_tab_bar_update(civ_tab_bar_t *tb, civ_input_state_t *input, float dt) {
  if (!tb || !input || !tb->base.visible) return;
  (void)dt;

  if (tb->tab_count == 0) return;
  tb->tab_width = tb->base.w / (float)tb->tab_count;

  if (input->mouse_left_pressed) {
    for (int i = 0; i < tb->tab_count; i++) {
      float tx = tb->base.x + (float)i * tb->tab_width;
      if (pt_in((float)input->mouse_x, (float)input->mouse_y,
                tx, tb->base.y, tb->tab_width, tb->base.h)) {
        tb->active_tab = i;
        break;
      }
    }
  }
}

void civ_tab_bar_render(civ_tab_bar_t *tb, SDL_Renderer *r) {
  if (!tb || !r || !tb->base.visible || tb->tab_count == 0) return;

  tb->tab_width = tb->base.w / (float)tb->tab_count;

  for (int i = 0; i < tb->tab_count; i++) {
    float tx = tb->base.x + (float)i * tb->tab_width;
    bool active = (i == tb->active_tab);

    uint32_t bg = active ? tb->color_active : tb->color_bg;
    uint8_t alpha = active ? 200 : 140;
    civ_render_rect_filled_alpha(r, (int)tx, (int)tb->base.y,
        (int)tb->tab_width, (int)tb->base.h, bg, alpha);
    civ_render_rect_outline(r, (int)tx, (int)tb->base.y,
        (int)tb->tab_width, (int)tb->base.h, tb->color_active, 1);

    /* The text label — caller renders via font system since we need font */
  }
}
