/**
 * @file graph.h
 * @brief Data visualization graph system — 20 chart types for game data
 *
 * All graphs render to SDL_Texture for composition into panels.
 * Shared infrastructure: axes, gridlines, labels, legends, color palettes.
 */
#ifndef CIV_UI_GRAPH_H
#define CIV_UI_GRAPH_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Color palettes ─────────────────────────────────────────────────── */
#define CIV_GRAPH_PALETTE_SIZE 12
extern const uint32_t g_graph_palette_default[CIV_GRAPH_PALETTE_SIZE];
extern const uint32_t g_graph_palette_terrain[CIV_GRAPH_PALETTE_SIZE];
extern const uint32_t g_graph_palette_warm[CIV_GRAPH_PALETTE_SIZE];
extern const uint32_t g_graph_palette_cool[CIV_GRAPH_PALETTE_SIZE];

/* ── Data structures ────────────────────────────────────────────────── */
typedef struct {
  const char *label;
  float       value;
  uint32_t    color;
} civ_graph_bar_t;

typedef struct {
  float *x_values;
  float *y_values;
  int    count;
  const char *label;
  uint32_t    color;
} civ_graph_series_t;

/* Multi-series for line/area charts */
typedef struct {
  civ_graph_series_t *series;
  int                 series_count;
  const char         *title;
  const char         *x_label;
  const char         *y_label;
} civ_graph_multi_series_t;

/* ── Graph context ───────────────────────────────────────────────────── */
typedef struct {
  int    x, y, w, h;
  float  x_min, x_max, y_min, y_max;
  bool   show_grid;
  bool   show_labels;
  int    font_size;
  uint32_t grid_color;
  uint32_t label_color;
  uint32_t bg_color;
  int    pad_top, pad_right, pad_bottom, pad_left;
} civ_graph_ctx_t;

void civ_graph_ctx_init(civ_graph_ctx_t *ctx, int x, int y, int w, int h);

/* ── Graph render functions (all return an SDL_Texture) ──────────────── */
SDL_Texture *civ_graph_line(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                            civ_graph_multi_series_t *data);
SDL_Texture *civ_graph_bar(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                           civ_graph_bar_t *bars, int count, bool horizontal);
SDL_Texture *civ_graph_pie(SDL_Renderer *r, int x, int y, int radius,
                           civ_graph_bar_t *slices, int count);
SDL_Texture *civ_graph_area(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                            civ_graph_multi_series_t *data);
SDL_Texture *civ_graph_scatter(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                               float *x, float *y, int count, uint32_t color,
                               float radius);
SDL_Texture *civ_graph_radar(SDL_Renderer *r, int cx, int cy, int radius,
                             const char **labels, float *values, int count,
                             uint32_t color);
SDL_Texture *civ_graph_gauge(SDL_Renderer *r, int cx, int cy, int radius,
                             float value, float min, float max,
                             uint32_t color);
SDL_Texture *civ_graph_sparkline(SDL_Renderer *r, int x, int y, int w, int h,
                                 float *values, int count, uint32_t color);

/* ── Axis helpers ────────────────────────────────────────────────────── */
void civ_graph_draw_axes(SDL_Renderer *r, civ_graph_ctx_t *ctx);
void civ_graph_draw_grid(SDL_Renderer *r, civ_graph_ctx_t *ctx,
                         int x_divisions, int y_divisions);
void civ_graph_map_point(civ_graph_ctx_t *ctx, float x_val, float y_val,
                         int *px, int *py);

#ifdef __cplusplus
}
#endif
#endif
