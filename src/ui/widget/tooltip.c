#include "ui/widget/tooltip.h"
#include "engine/renderer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

civ_tooltip_t *civ_tooltip_create(const char *id) {
  civ_tooltip_t *t = (civ_tooltip_t *)malloc(sizeof(civ_tooltip_t));
  if (!t) return NULL;
  memset(t, 0, sizeof(*t));
  civ_widget_init(&t->base, CIV_WIDGET_PANEL, id, 0, 0, 260, 40);
  t->pad = 8.0f;
  t->line_height = 18.0f;
  t->font_size = 14;
  t->offset_x = 16.0f;
  t->offset_y = 16.0f;
  t->target_alpha = 0.0f;
  civ_tween_init(&t->fade);
  t->base.visible = false;
  return t;
}

void civ_tooltip_destroy(civ_tooltip_t *t) { free(t); }

void civ_tooltip_clear(civ_tooltip_t *t) {
  if (!t) return;
  t->line_count = 0;
}

civ_tooltip_t *civ_tooltip_add_header(civ_tooltip_t *t, const char *text,
                                      uint32_t color) {
  if (!t || t->line_count >= CIV_TOOLTIP_LINES_MAX) return t;
  civ_tooltip_line_t *l = &t->lines[t->line_count++];
  snprintf(l->text, CIV_TOOLTIP_LINE_LEN, "%s", text ? text : "");
  l->color = color ? color : g_theme.text_primary;
  l->is_header = true;
  l->separator_before = false;
  return t;
}

civ_tooltip_t *civ_tooltip_add_line(civ_tooltip_t *t, const char *text,
                                    uint32_t color) {
  if (!t || t->line_count >= CIV_TOOLTIP_LINES_MAX) return t;
  civ_tooltip_line_t *l = &t->lines[t->line_count++];
  snprintf(l->text, CIV_TOOLTIP_LINE_LEN, "  %s", text ? text : "");
  l->color = color ? color : g_theme.text_secondary;
  l->is_header = false;
  l->separator_before = false;
  return t;
}

civ_tooltip_t *civ_tooltip_add_separator(civ_tooltip_t *t) {
  if (!t || t->line_count >= CIV_TOOLTIP_LINES_MAX) return t;
  civ_tooltip_line_t *l = &t->lines[t->line_count++];
  l->text[0] = '\0';
  l->color = 0x1A2A3A;
  l->is_header = false;
  l->separator_before = true;
  return t;
}

civ_tooltip_t *civ_tooltip_add_key_value(civ_tooltip_t *t, const char *key,
                                          const char *value,
                                          uint32_t key_color,
                                          uint32_t val_color) {
  if (!t || t->line_count >= CIV_TOOLTIP_LINES_MAX) return t;
  civ_tooltip_line_t *l = &t->lines[t->line_count++];
  snprintf(l->text, CIV_TOOLTIP_LINE_LEN, "%-24s %s", key ? key : "",
           value ? value : "");
  l->color = 0; /* Special: render as two colors (key + value) */
  l->is_header = false;
  l->separator_before = false;
  (void)key_color; (void)val_color;
  return t;
}

civ_tooltip_t *civ_tooltip_add_bar(civ_tooltip_t *t, const char *label,
                                    float value, uint32_t bar_color) {
  if (!t || t->line_count >= CIV_TOOLTIP_LINES_MAX) return t;
  civ_tooltip_line_t *l = &t->lines[t->line_count++];
  snprintf(l->text, CIV_TOOLTIP_LINE_LEN, "BAR:%.2f:%s",
           value, label ? label : "");
  l->color = bar_color ? bar_color : g_theme.primary;
  l->is_header = false;
  l->separator_before = false;
  return t;
}

void civ_tooltip_show(civ_tooltip_t *t, float cx, float cy) {
  if (!t) return;
  t->cursor_x = cx;
  t->cursor_y = cy;
  t->pending_show = true;
}

void civ_tooltip_hide(civ_tooltip_t *t) {
  if (!t) return;
  t->pending_show = false;
  t->target_alpha = 0.0f;
}

void civ_tooltip_update(civ_tooltip_t *t, civ_input_state_t *input, float dt,
                        int win_w, int win_h) {
  if (!t) return;

  if (t->pending_show) {
    t->pending_show = false;
    t->target_alpha = 1.0f;
    if (!t->base.visible) {
      t->base.visible = true;
      civ_tween_start(&t->fade, CIV_TWEEN_FADE_IN, 0.12f,
                      civ_tween_value(&t->fade), 1.0f);
    }
  }

  civ_tween_update(&t->fade, dt);
  float alpha = civ_tween_value(&t->fade);

  if (t->target_alpha < 0.01f && alpha < 0.02f) {
    t->base.visible = false;
    return;
  }

  /* Compute size from content */
  float h = t->pad * 2.0f + (float)t->line_count * t->line_height;
  float max_w = 0.0f;
  for (int i = 0; i < t->line_count; i++) {
    float lw = (float)strlen(t->lines[i].text) * ((float)t->font_size * 0.55f);
    if (lw > max_w) max_w = lw;
  }
  float w = max_w + t->pad * 2.0f;
  if (w < 180.0f) w = 180.0f;
  if (w > 400.0f) w = 400.0f;

  t->base.w = w;
  t->base.h = h;

  /* Smart positioning: avoid screen edges */
  float tx = t->cursor_x + t->offset_x;
  float ty = t->cursor_y + t->offset_y;
  if (tx + w > (float)win_w - 8.0f) tx = t->cursor_x - w - t->offset_x;
  if (ty + h > (float)win_h - 8.0f) ty = t->cursor_y - h - t->offset_y;
  if (tx < 4.0f) tx = 4.0f;
  if (ty < 4.0f) ty = 4.0f;
  t->base.x = tx;
  t->base.y = ty;

  (void)input;
}

void civ_tooltip_render(civ_tooltip_t *t, SDL_Renderer *r, civ_font_t *font) {
  if (!t || !r || !t->base.visible) return;

  float alpha = civ_tween_value(&t->fade);
  if (alpha < 0.02f) return;

  int x = (int)t->base.x, y = (int)t->base.y;
  int w = (int)t->base.w, h = (int)t->base.h;

  /* Background with border */
  civ_render_rect_filled_alpha(r, x, y, w, h, g_theme.bg_medium,
                               (uint8_t)(230 * alpha));
  civ_render_rect_outline(r, x, y, w, h, g_theme.primary, 1);

  /* Render each line */
  float ly = (float)y + t->pad;
  for (int i = 0; i < t->line_count; i++) {
    civ_tooltip_line_t *l = &t->lines[i];

    if (l->separator_before && l->text[0] == '\0') {
      civ_render_line(r, x + (int)t->pad, (int)(ly + t->line_height / 2.0f),
                      x + w - (int)t->pad, (int)(ly + t->line_height / 2.0f),
                      0x1A2A3A);
      ly += t->line_height;
      continue;
    }

    if (l->text[0] == 'B' && l->text[1] == 'A' && l->text[2] == 'R' &&
        l->text[3] == ':') {
      /* Inline bar */
      float bar_val = 0.0f;
      char bar_label[64] = "";
      sscanf(l->text, "BAR:%f:%63s", &bar_val, bar_label);
      if (font)
        civ_font_render_aligned(r, font, bar_label, x + (int)t->pad, (int)ly,
                                100, (int)t->line_height, l->color,
                                CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
      int bar_x = x + (int)t->pad + 110;
      int bar_w = w - (int)t->pad * 2 - 110;
      civ_render_rect_filled(r, bar_x, (int)(ly + 4), bar_w,
                             (int)t->line_height - 8, 0x1A2A3A);
      civ_render_rect_filled(r, bar_x, (int)(ly + 4),
                             (int)((float)bar_w * bar_val),
                             (int)t->line_height - 8, l->color);
      ly += t->line_height;
      continue;
    }

    if (font) {
      uint32_t tc = l->color ? l->color : g_theme.text_secondary;
      int fs = l->is_header ? t->font_size + 2 : t->font_size;
      /* Use a simple text render at the correct y */
      civ_font_render_aligned(r, font, l->text, x + (int)t->pad, (int)ly,
                              w - (int)t->pad * 2, fs, tc, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
    }
    ly += t->line_height;
  }
}
