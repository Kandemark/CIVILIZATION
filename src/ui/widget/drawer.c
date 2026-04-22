#include "ui/widget/drawer.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_drawer_t *civ_drawer_create(const char *id, bool right_side,
                                float open_width, const char *label,
                                civ_icon_id_t icon) {
  civ_drawer_t *d = (civ_drawer_t *)malloc(sizeof(civ_drawer_t));
  if (!d) return NULL;
  memset(d, 0, sizeof(*d));

  d->right_side = right_side;
  d->is_open = false;
  d->is_pinned = false;
  d->collapsed_w = 28.0f;   /* thin tab strip */
  d->open_w = open_width;
  d->current_w = d->collapsed_w;
  d->min_w = 180.0f;
  d->max_w = 500.0f;
  d->tab_width = 28;
  d->resize_handle_w = 5;
  d->tab_icon = icon;
  d->tab_color = 0x1A2A3A;
  d->tab_color_hover = g_theme.primary;
  d->color_bg = g_theme.bg_dark;
  d->color_border = g_theme.primary;
  d->border_width = 1;
  if (label) strncpy(d->tab_label, label, sizeof(d->tab_label) - 1);

  civ_tween_init(&d->width_tween);
  civ_tween_init(&d->alpha_tween);

  civ_widget_init(&d->base, CIV_WIDGET_PANEL, id, 0, 0, d->current_w, 0);
  return d;
}

void civ_drawer_destroy(civ_drawer_t *d) {
  if (!d) return;
  civ_widget_destroy_children(&d->base);
  free(d);
}

void civ_drawer_open(civ_drawer_t *d) {
  if (!d || d->is_open) return;
  d->is_open = true;
  civ_tween_start(&d->width_tween, CIV_TWEEN_FADE_IN, g_theme.anim_slide_ms,
                  d->current_w, d->open_w);
  civ_tween_start(&d->alpha_tween, CIV_TWEEN_FADE_IN, g_theme.anim_fade_ms,
                  0.3f, 1.0f);
}

void civ_drawer_close(civ_drawer_t *d) {
  if (!d || !d->is_open) return;
  d->is_open = false;
  civ_tween_start(&d->width_tween, CIV_TWEEN_FADE_OUT, g_theme.anim_slide_ms,
                  d->current_w, d->collapsed_w);
  civ_tween_start(&d->alpha_tween, CIV_TWEEN_FADE_OUT, g_theme.anim_fade_ms,
                  1.0f, 0.3f);
}

void civ_drawer_toggle(civ_drawer_t *d) {
  if (!d) return;
  if (d->is_open) civ_drawer_close(d);
  else civ_drawer_open(d);
}

bool civ_drawer_is_open(const civ_drawer_t *d) { return d ? d->is_open : false; }

void civ_drawer_get_content_rect(civ_drawer_t *d, int win_h,
                                 float *x, float *y, float *w, float *h) {
  float tab_w = (float)d->tab_width;
  if (d->right_side) {
    *x = (float)d->base.x + tab_w;
  } else {
    *x = (float)d->base.x + tab_w;
  }
  *y = 40.0f;  /* below top bar */
  *w = d->current_w - tab_w - (float)d->border_width;
  *h = (float)win_h - *y;
}

void civ_drawer_update(civ_drawer_t *d, civ_input_state_t *input,
                       float dt, int win_w, int win_h) {
  if (!d || !input) return;

  /* Animate width */
  civ_tween_update(&d->width_tween, dt);
  civ_tween_update(&d->alpha_tween, dt);
  d->current_w = civ_tween_value(&d->width_tween);

  /* Position: anchored to edge */
  if (d->right_side) {
    d->base.x = (float)win_w - d->current_w;
  } else {
    d->base.x = 0;
  }
  d->base.y = 0;
  d->base.w = d->current_w;
  d->base.h = (float)win_h;

  /* Tab hit area */
  float tab_x = d->right_side ? d->base.x : d->base.x + d->current_w - (float)d->tab_width;
  d->hover_tab = civ_input_is_mouse_over(input, (int)tab_x, 0, d->tab_width, win_h);

  /* Click tab to toggle */
  if (d->hover_tab && input->mouse_left_pressed) {
    civ_drawer_toggle(d);
    return;
  }

  /* Resize handle (inner edge of drawer when open) */
  if (d->is_open && !d->is_resizing) {
    float handle_x = d->right_side
        ? d->base.x + (float)d->resize_handle_w
        : d->base.x + d->current_w - (float)d->resize_handle_w;
    if (civ_input_is_mouse_over(input, (int)(handle_x - (float)d->resize_handle_w),
                                0, d->resize_handle_w * 3, win_h)) {
      if (input->mouse_left_pressed) {
        d->is_resizing = true;
        d->resize_grab_x = (float)input->mouse_x;
        d->resize_start_w = d->current_w;
      }
    }
  }

  if (d->is_resizing) {
    if (input->mouse_left_down) {
      float dx = (float)input->mouse_x - d->resize_grab_x;
      float new_w = d->right_side
          ? d->resize_start_w - dx
          : d->resize_start_w + dx;
      if (new_w < d->min_w) new_w = d->min_w;
      if (new_w > d->max_w) new_w = d->max_w;
      d->open_w = new_w;
      if (d->is_open && !d->width_tween.active) d->current_w = new_w;
    } else {
      d->is_resizing = false;
    }
  }

  /* Keyboard: ESC closes */
  if (input->esc_pressed && d->is_open) {
    civ_drawer_close(d);
  }

  (void)win_w;
}

void civ_drawer_render_chrome(civ_drawer_t *d, SDL_Renderer *r,
                              int win_w, int win_h) {
  if (!d || !r) return;

  float alpha = civ_tween_value(&d->alpha_tween);
  if (alpha < 0.01f && !d->is_open) return;

  int x = (int)d->base.x, w = (int)d->current_w;

  /* Background */
  civ_render_rect_filled_alpha(r, x, 0, w, win_h, d->color_bg,
                               (uint8_t)(220 * alpha));

  /* Right border (or left for left-side drawer) */
  if (d->right_side) {
    civ_render_line(r, x, 0, x, win_h, d->color_border);
  } else {
    civ_render_line(r, x + w, 0, x + w, win_h, d->color_border);
  }

  /* Resize handle visible when open */
  if (d->is_open && alpha > 0.5f) {
    int hx = d->right_side ? x + d->resize_handle_w / 2 : x + w - d->resize_handle_w / 2;
    civ_render_rect_filled_alpha(r, hx - 1, 0, 3, win_h,
                                  d->is_resizing ? d->color_border : 0x1A2A3A,
                                  120);
  }

  /* Tab strip — bright and clearly visible */
  float tab_x = d->right_side ? (float)x : (float)(x + w - d->tab_width);
  uint32_t tab_c = d->hover_tab ? d->tab_color_hover : d->tab_color;
  civ_render_rect_filled_alpha(r, (int)tab_x, 0, d->tab_width, win_h, tab_c, 240);
  /* Vertical label in the middle of the tab */
  /* Render a few dots/lines as a visual grip */
  for (int dot_y = win_h/2 - 20; dot_y <= win_h/2 + 20; dot_y += 8) {
    civ_render_line(r, (int)(tab_x + 10), dot_y, (int)(tab_x + 22), dot_y,
                    d->hover_tab ? 0xFFFFFF : 0x8899AA);
  }
  (void)win_w;
}
