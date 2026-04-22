#include "ui/widget/panel.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_panel_t *civ_panel_create(const char *id, float x, float y, float w, float h,
                              const char *title) {
  civ_panel_t *p = (civ_panel_t *)malloc(sizeof(civ_panel_t));
  if (!p) return NULL;
  memset(p, 0, sizeof(*p));
  civ_widget_init(&p->base, CIV_WIDGET_PANEL, id, x, y, w, h);
  if (title) strncpy(p->title, title, sizeof(p->title) - 1);
  p->draggable = true;
  p->closable = true;
  p->is_open = false;
  p->color_bg = g_theme.bg_dark;
  p->color_border = g_theme.primary;
  p->border_width = g_theme.panel_border_width;
  p->corner_radius = g_theme.panel_border_radius;
  civ_panel_anim_init(&p->anim);
  return p;
}

void civ_panel_destroy(civ_panel_t *p) {
  if (p) civ_widget_destroy_children(&p->base);
  free(p);
}

void civ_panel_open(civ_panel_t *p) {
  if (!p || p->is_open) return;
  p->is_open = true;
  civ_panel_anim_show(&p->anim, g_theme.anim_fade_ms);
}

void civ_panel_close(civ_panel_t *p) {
  if (!p || !p->is_open) return;
  p->is_open = false;
  civ_panel_anim_hide(&p->anim, g_theme.anim_fade_ms);
}

void civ_panel_toggle(civ_panel_t *p) {
  if (!p) return;
  if (p->is_open) civ_panel_close(p);
  else civ_panel_open(p);
}

void civ_panel_update(civ_panel_t *p, civ_input_state_t *input, float dt) {
  if (!p || !input) return;
  civ_panel_anim_update(&p->anim, dt);

  if (!civ_panel_anim_is_visible(&p->anim)) {
    p->base.visible = false;
    return;
  }
  p->base.visible = true;

  /* Close on ESC */
  if (input->esc_pressed && p->closable && p->is_open) {
    civ_panel_close(p);
    input->esc_pressed = false;
    return;
  }

  /* Drag handling on title bar */
  if (p->draggable) {
    int title_bar_h = 30;
    bool in_title =
        civ_input_is_mouse_over(input, (int)p->base.x, (int)p->base.y,
                                (int)p->base.w, title_bar_h);

    if (in_title && input->mouse_left_pressed) {
      p->dragging = true;
      p->drag_offset_x = (float)input->mouse_x - p->base.x;
      p->drag_offset_y = (float)input->mouse_y - p->base.y;
    }
    if (p->dragging && input->mouse_left_down) {
      p->base.x = (float)input->mouse_x - p->drag_offset_x;
      p->base.y = (float)input->mouse_y - p->drag_offset_y;
    }
    if (!input->mouse_left_down) {
      p->dragging = false;
    }
  }
}

void civ_panel_render_chrome(civ_panel_t *p, SDL_Renderer *r, int win_w,
                             int win_h) {
  if (!p || !p->base.visible || !r) return;

  float alpha = civ_panel_anim_alpha(&p->anim);
  if (alpha <= 0.01f) return;

  float ox = civ_panel_anim_offset_x(&p->anim);
  float oy = civ_panel_anim_offset_y(&p->anim);
  float sc = civ_panel_anim_scale(&p->anim);

  float cx = p->base.x + p->base.w / 2.0f;
  float cy = p->base.y + p->base.h / 2.0f;
  float rw = p->base.w * sc;
  float rh = p->base.h * sc;
  float rx = cx - rw / 2.0f + ox;
  float ry = cy - rh / 2.0f + oy;

  /* Background */
  civ_render_rect_filled_alpha(r, (int)rx, (int)ry, (int)rw, (int)rh,
                               p->color_bg, (uint8_t)(220 * alpha));

  /* Border */
  civ_render_rect_outline(r, (int)rx, (int)ry, (int)rw, (int)rh, p->color_border,
                          p->border_width);

  /* Title bar */
  int title_h = 30;
  civ_render_rect_filled_alpha(r, (int)rx, (int)ry, (int)rw, title_h,
                               g_theme.bg_medium, (uint8_t)(240 * alpha));
  civ_render_line(r, (int)rx, (int)(ry + (float)title_h), (int)(rx + rw),
                  (int)(ry + (float)title_h), p->color_border);

  /* Title text placeholder — rendered by owner via font system */
  (void)win_w;
  (void)win_h;
}

void civ_panel_render(civ_panel_t *p, SDL_Renderer *r) {
  civ_panel_render_chrome(p, r, 0, 0);
}
