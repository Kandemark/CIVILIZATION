#include "../../../include/ui/graph/graph.h"
#include "../../../include/engine/renderer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Palettes ───────────────────────────────────────────────────────── */
const uint32_t g_graph_palette_default[12] = {
    0x00D4FF, 0xFFD400, 0x00FFD2, 0xFF4A6A, 0x9B59B6, 0x3498DB,
    0xE67E22, 0x2ECC71, 0xF1C40F, 0xE74C3C, 0x1ABC9C, 0x95A5A6};
const uint32_t g_graph_palette_terrain[12] = {
    0x2ECC71, 0xF1C40F, 0xE67E22, 0x795548, 0x3498DB, 0x1ABC9C,
    0x8BC34A, 0xFF9800, 0x607D8B, 0x4CAF50, 0x2196F3, 0x9E9E9E};
const uint32_t g_graph_palette_warm[12] = {
    0xFF6B6B, 0xFFA07A, 0xFFD700, 0xFF8C00, 0xFF4500, 0xDC143C,
    0xFF69B4, 0xFF1493, 0xFFA500, 0xFFDAB9, 0xFF6347, 0xFF7F50};
const uint32_t g_graph_palette_cool[12] = {
    0x00CED1, 0x4682B4, 0x5F9EA0, 0x6495ED, 0x7B68EE, 0x6A5ACD,
    0x00BFFF, 0x1E90FF, 0x00FA9A, 0x3CB371, 0x20B2AA, 0x48D1CC};

/* ── Context ────────────────────────────────────────────────────────── */
void civ_graph_ctx_init(civ_graph_ctx_t *ctx, int x, int y, int w, int h) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->x = x; ctx->y = y; ctx->w = w; ctx->h = h;
  ctx->show_grid = true;
  ctx->show_labels = true;
  ctx->font_size = 12;
  ctx->grid_color = 0x1A2A3A;
  ctx->label_color = 0xA0B4C8;
  ctx->bg_color = 0x070B1A;
  ctx->pad_top = 10; ctx->pad_right = 10; ctx->pad_bottom = 30; ctx->pad_left = 40;
}

void civ_graph_map_point(civ_graph_ctx_t *ctx, float x_val, float y_val,
                         int *px, int *py) {
  float plot_w = (float)(ctx->w - ctx->pad_left - ctx->pad_right);
  float plot_h = (float)(ctx->h - ctx->pad_top - ctx->pad_bottom);
  float x_range = ctx->x_max - ctx->x_min;
  float y_range = ctx->y_max - ctx->y_min;
  *px = ctx->x + ctx->pad_left +
        (int)((x_val - ctx->x_min) / (x_range > 0 ? x_range : 1.0f) * plot_w);
  *py = ctx->y + ctx->pad_top +
        (int)((1.0f - (y_val - ctx->y_min) / (y_range > 0 ? y_range : 1.0f)) *
              plot_h);
}

void civ_graph_draw_axes(SDL_Renderer *r, civ_graph_ctx_t *ctx) {
  int plot_x = ctx->x + ctx->pad_left, plot_y = ctx->y + ctx->pad_top;
  int plot_w = ctx->w - ctx->pad_left - ctx->pad_right;
  int plot_h = ctx->h - ctx->pad_top - ctx->pad_bottom;
  civ_render_line(r, plot_x, plot_y, plot_x, plot_y + plot_h, ctx->grid_color);
  civ_render_line(r, plot_x, plot_y + plot_h, plot_x + plot_w, plot_y + plot_h,
                  ctx->grid_color);
}

void civ_graph_draw_grid(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                         int x_divs, int y_divs) {
  int plot_x = ctx->x + ctx->pad_left, plot_y = ctx->y + ctx->pad_top;
  int plot_w = ctx->w - ctx->pad_left - ctx->pad_right;
  int plot_h = ctx->h - ctx->pad_top - ctx->pad_bottom;
  for (int i = 0; i <= x_divs; i++) {
    int x = plot_x + (plot_w * i) / (x_divs > 0 ? x_divs : 1);
    civ_render_line(r, x, plot_y, x, plot_y + plot_h, ctx->grid_color);
  }
  for (int i = 0; i <= y_divs; i++) {
    int y = plot_y + (plot_h * i) / (y_divs > 0 ? y_divs : 1);
    civ_render_line(r, plot_x, y, plot_x + plot_w, y, ctx->grid_color);
  }
}

/* ── Line chart ──────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_line(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                            civ_graph_multi_series_t *data) {
  if (!r || !data || data->series_count == 0) return NULL;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, ctx->w, ctx->h);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, ctx->w, ctx->h, ctx->bg_color);
  civ_graph_draw_grid(r, ctx, 5, 5);
  civ_graph_draw_axes(r, ctx);

  for (int s = 0; s < data->series_count; s++) {
    civ_graph_series_t *ser = &data->series[s];
    if (ser->count < 2) continue;
    uint32_t col = ser->color ? ser->color
                              : g_graph_palette_default[s % CIV_GRAPH_PALETTE_SIZE];
    int px0, py0;
    civ_graph_map_point(ctx, ser->x_values[0], ser->y_values[0], &px0, &py0);
    for (int i = 1; i < ser->count; i++) {
      int px1, py1;
      civ_graph_map_point(ctx, ser->x_values[i], ser->y_values[i], &px1, &py1);
      civ_render_line(r, px0, py0, px1, py1, col);
      px0 = px1; py0 = py1;
    }
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Bar chart ───────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_bar(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                           civ_graph_bar_t *bars, int count, bool horizontal) {
  if (!r || !bars || count == 0) return NULL;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, ctx->w, ctx->h);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, ctx->w, ctx->h, ctx->bg_color);
  civ_graph_draw_axes(r, ctx);

  float max_val = 0.0f;
  for (int i = 0; i < count; i++)
    if (bars[i].value > max_val) max_val = bars[i].value;
  if (max_val <= 0.0f) max_val = 1.0f;

  int plot_x = ctx->x + ctx->pad_left, plot_y = ctx->y + ctx->pad_top;
  int plot_w = ctx->w - ctx->pad_left - ctx->pad_right;
  int plot_h = ctx->h - ctx->pad_top - ctx->pad_bottom;

  if (horizontal) {
    for (int i = 0; i < count; i++) {
      int bar_h = (plot_h / count) - 4;
      int by = plot_y + (plot_h * i) / count + 2;
      int bw = (int)((bars[i].value / max_val) * (float)plot_w);
      uint32_t col = bars[i].color ? bars[i].color
                                   : g_graph_palette_default[i % CIV_GRAPH_PALETTE_SIZE];
      civ_render_rect_filled(r, plot_x, by, bw, bar_h, col);
    }
  } else {
    for (int i = 0; i < count; i++) {
      int bar_w = (plot_w / count) - 4;
      int bx = plot_x + (plot_w * i) / count + 2;
      int bh = (int)((bars[i].value / max_val) * (float)plot_h);
      int by = plot_y + plot_h - bh;
      uint32_t col = bars[i].color ? bars[i].color
                                   : g_graph_palette_default[i % CIV_GRAPH_PALETTE_SIZE];
      civ_render_rect_filled(r, bx, by, bar_w, bh, col);
    }
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Pie chart ───────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_pie(SDL_Renderer *r, int x, int y, int radius,
                           civ_graph_bar_t *slices, int count) {
  if (!r || !slices || count == 0) return NULL;
  int size = (radius + 20) * 2;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, size, size);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, size, size, 0x00000000);

  float total = 0.0f;
  for (int i = 0; i < count; i++) total += slices[i].value;
  if (total <= 0.0f) { SDL_SetRenderTarget(r, NULL); return tex; }

  int cx = size / 2, cy = size / 2;
  float angle = -M_PI / 2.0f;
  for (int i = 0; i < count; i++) {
    float sweep = (slices[i].value / total) * 2.0f * (float)M_PI;
    uint32_t col = slices[i].color ? slices[i].color
                                   : g_graph_palette_default[i % CIV_GRAPH_PALETTE_SIZE];

    SDL_SetRenderDrawColor(r, (col >> 16) & 0xFF, (col >> 8) & 0xFF,
                           col & 0xFF, 255);
    /* Approximate pie slice as many triangles */
    int segments = (int)(sweep * 20.0f) + 3;
    float step = sweep / (float)segments;
    for (int j = 0; j < segments; j++) {
      float a1 = angle + (float)j * step, a2 = angle + (float)(j + 1) * step;
      SDL_FPoint pts[] = {{(float)cx, (float)cy},
                          {(float)cx + cosf(a1) * (float)radius,
                           (float)cy + sinf(a1) * (float)radius},
                          {(float)cx + cosf(a2) * (float)radius,
                           (float)cy + sinf(a2) * (float)radius}};
      /* SDL_RenderGeometry not available in SDL3 Render API, use line fill */
      for (int k = 0; k < (int)(cosf(a1) * radius); k++)
        civ_render_line(r, cx, cy,
                        (int)(cx + cosf(a1 + step * 0.5f) * radius),
                        (int)(cy + sinf(a1 + step * 0.5f) * radius), col);
    }
    angle += sweep;
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Area chart ──────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_area(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                            civ_graph_multi_series_t *data) {
  return civ_graph_line(r, ctx, data);
}

/* ── Scatter ─────────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_scatter(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                               float *x, float *y, int count, uint32_t color,
                               float radius) {
  if (!r || !x || !y || count == 0) return NULL;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, ctx->w, ctx->h);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, ctx->w, ctx->h, ctx->bg_color);
  civ_graph_draw_axes(r, ctx);

  int ir = (int)(radius > 0.0f ? radius : 3.0f);
  for (int i = 0; i < count; i++) {
    int px, py;
    civ_graph_map_point(ctx, x[i], y[i], &px, &py);
    civ_render_rect_filled(r, px - ir, py - ir, ir * 2, ir * 2, color);
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Radar / spider chart ────────────────────────────────────────────── */
SDL_Texture *civ_graph_radar(SDL_Renderer *r, int cx, int cy, int radius,
                             const char **labels, float *values, int count,
                             uint32_t color) {
  if (!r || !values || count < 3) return NULL;
  int size = (radius + 60) * 2;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, size, size);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, size, size, 0x070B1A);

  int o_cx = size / 2, o_cy = size / 2;
  float angle_step = 2.0f * (float)M_PI / (float)count;

  /* Grid rings */
  for (int ring = 1; ring <= 4; ring++) {
    float ring_r = (float)radius * (float)ring / 4.0f;
    int px0 = o_cx + (int)(cosf(-M_PI / 2.0f) * ring_r);
    int py0 = o_cy + (int)(sinf(-M_PI / 2.0f) * ring_r);
    for (int i = 1; i <= count; i++) {
      float a = -M_PI / 2.0f + angle_step * (float)i;
      int px1 = o_cx + (int)(cosf(a) * ring_r), py1 = o_cy + (int)(sinf(a) * ring_r);
      civ_render_line(r, px0, py0, px1, py1, 0x1A2A3A);
      px0 = px1; py0 = py1;
    }
  }

  /* Axes */
  for (int i = 0; i < count; i++) {
    float a = -M_PI / 2.0f + angle_step * (float)i;
    int ax = o_cx + (int)(cosf(a) * (float)radius);
    int ay = o_cy + (int)(sinf(a) * (float)radius);
    civ_render_line(r, o_cx, o_cy, ax, ay, 0x1A2A3A);
  }

  /* Data polygon */
  int pts_x[16], pts_y[16];
  for (int i = 0; i < count; i++) {
    float a = -M_PI / 2.0f + angle_step * (float)i;
    float clamped = values[i] > 1.0f ? 1.0f : (values[i] < 0.0f ? 0.0f : values[i]);
    pts_x[i] = o_cx + (int)(cosf(a) * (float)radius * clamped);
    pts_y[i] = o_cy + (int)(sinf(a) * (float)radius * clamped);
  }
  for (int i = 0; i < count; i++) {
    int j = (i + 1) % count;
    civ_render_line(r, pts_x[i], pts_y[i], pts_x[j], pts_y[j], color);
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Gauge ───────────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_gauge(SDL_Renderer *r, int cx, int cy, int radius,
                             float value, float min, float max, uint32_t color) {
  if (!r) return NULL;
  int size = (radius + 10) * 2;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, size, size);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, size, size, 0x00000000);

  float pct = (value - min) / (max - min);
  if (pct < 0.0f) pct = 0.0f; if (pct > 1.0f) pct = 1.0f;

  /* Background arc */
  for (int i = 0; i < 36; i++) {
    float a1 = M_PI + (float)i * M_PI / 36.0f;
    float a2 = M_PI + (float)(i + 1) * M_PI / 36.0f;
    civ_render_line(r, (int)(cx + cosf(a1) * radius), (int)(cy + sinf(a1) * radius),
                    (int)(cx + cosf(a2) * radius), (int)(cy + sinf(a2) * radius),
                    0x1A2A3A);
  }
  /* Value arc */
  int ticks = (int)(pct * 36.0f);
  for (int i = 0; i < ticks; i++) {
    float a1 = M_PI + (float)i * M_PI / 36.0f;
    float a2 = M_PI + (float)(i + 1) * M_PI / 36.0f;
    civ_render_line(r, (int)(cx + cosf(a1) * radius * 0.8f),
                    (int)(cy + sinf(a1) * radius * 0.8f),
                    (int)(cx + cosf(a2) * radius * 0.8f),
                    (int)(cy + sinf(a2) * radius * 0.8f), color);
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}

/* ── Sparkline ───────────────────────────────────────────────────────── */
SDL_Texture *civ_graph_sparkline(SDL_Renderer *r, int x, int y, int w, int h,
                                 float *values, int count, uint32_t color) {
  if (!r || !values || count < 2) return NULL;
  SDL_Texture *tex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET, w, h);
  if (!tex) return NULL;
  SDL_SetRenderTarget(r, tex);
  civ_render_rect_filled(r, 0, 0, w, h, 0x00000000);

  float min_v = values[0], max_v = values[0];
  for (int i = 1; i < count; i++) {
    if (values[i] < min_v) min_v = values[i];
    if (values[i] > max_v) max_v = values[i];
  }
  float range = max_v - min_v;
  if (range <= 0.0f) range = 1.0f;

  int px0 = 0, py0 = h - 1 - (int)((values[0] - min_v) / range * (float)(h - 1));
  for (int i = 1; i < count; i++) {
    int px1 = (int)((float)i / (float)(count - 1) * (float)(w - 1));
    int py1 = h - 1 - (int)((values[i] - min_v) / range * (float)(h - 1));
    civ_render_line(r, px0, py0, px1, py1, color);
    px0 = px1; py0 = py1;
  }
  SDL_SetRenderTarget(r, NULL);
  return tex;
}
