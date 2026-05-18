/**
 * @file layout.h
 * @brief Flexbox and grid layout engines for widget positioning
 *
 * Avoids hardcoded pixel positions. Panels use layouts to position children.
 * All coordinates are relative to their parent container.
 */
#ifndef CIV_UI_LAYOUT_H
#define CIV_UI_LAYOUT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Flex layout ────────────────────────────────────────────────────── */
typedef enum {
  CIV_FLEX_ROW,
  CIV_FLEX_COLUMN,
} civ_flex_dir_t;

typedef enum {
  CIV_ALIGN_START,
  CIV_ALIGN_CENTER,
  CIV_ALIGN_END,
  CIV_ALIGN_STRETCH,
} civ_align_t;

typedef enum {
  CIV_JUSTIFY_START,
  CIV_JUSTIFY_CENTER,
  CIV_JUSTIFY_END,
  CIV_JUSTIFY_SPACE_BETWEEN,
  CIV_JUSTIFY_SPACE_AROUND,
} civ_justify_t;

typedef struct {
  civ_flex_dir_t direction;
  civ_align_t    align;
  civ_justify_t  justify;
  float          gap;
  float          pad_top, pad_right, pad_bottom, pad_left;
  float          container_x, container_y, container_w, container_h;
} civ_flex_layout_t;

void civ_flex_init(civ_flex_layout_t *fl, float x, float y, float w, float h,
                   civ_flex_dir_t dir);

/* Compute the position for one child. Returns the total size consumed.
 * Call repeatedly for each child; pass child_w/h for the next item. */
void civ_flex_next(civ_flex_layout_t *fl, float child_w, float child_h,
                   float *out_x, float *out_y, float *out_w, float *out_h);

/* Convenience: layout an array of equal-sized items, returns count laid out */
int civ_flex_layout_row(civ_flex_layout_t *fl, float item_w, float item_h,
                        float **out_positions);

/* ── Grid layout ─────────────────────────────────────────────────────── */
typedef struct {
  int columns, rows;
  float gap;
  float pad_top, pad_right, pad_bottom, pad_left;
  float container_x, container_y, container_w, container_h;
  float *col_widths;  /* optional per-column widths */
  float *row_heights; /* optional per-row heights */
} civ_grid_layout_t;

void civ_grid_init(civ_grid_layout_t *gl, float x, float y, float w, float h,
                   int cols, int rows);
void civ_grid_cell(civ_grid_layout_t *gl, int col, int row, int col_span,
                   int row_span, float *out_x, float *out_y, float *out_w,
                   float *out_h);

#ifdef __cplusplus
}
#endif
#endif
