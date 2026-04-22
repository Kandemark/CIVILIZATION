#include "ui/layout/layout.h"
#include <stdlib.h>
#include <string.h>

/* ── Flex layout ────────────────────────────────────────────────────── */
void civ_flex_init(civ_flex_layout_t *fl, float x, float y, float w, float h,
                   civ_flex_dir_t dir) {
  memset(fl, 0, sizeof(*fl));
  fl->container_x = x; fl->container_y = y;
  fl->container_w = w; fl->container_h = h;
  fl->direction = dir;
  fl->align = CIV_ALIGN_START;
  fl->justify = CIV_JUSTIFY_START;
  fl->gap = 8.0f;
  fl->pad_left = fl->pad_right = fl->pad_top = fl->pad_bottom = 0.0f;
}

void civ_flex_next(civ_flex_layout_t *fl, float child_w, float child_h,
                   float *ox, float *oy, float *ow, float *oh) {
  static float cursor_x, cursor_y;
  static bool first = true;
  if (first) { cursor_x = fl->container_x + fl->pad_left;
               cursor_y = fl->container_y + fl->pad_top; first = false; }

  float avail_w = fl->container_w - fl->pad_left - fl->pad_right;
  float avail_h = fl->container_h - fl->pad_top - fl->pad_bottom;

  if (fl->direction == CIV_FLEX_ROW) {
    if (cursor_x + child_w > fl->container_x + fl->pad_left + avail_w) {
      cursor_x = fl->container_x + fl->pad_left;
      cursor_y += child_h + fl->gap;
    }
    *ox = cursor_x; *oy = cursor_y;
    *ow = (fl->align == CIV_ALIGN_STRETCH) ? child_w : child_w;
    *oh = (fl->align == CIV_ALIGN_STRETCH) ? avail_h - (cursor_y - fl->container_y - fl->pad_top) : child_h;
    cursor_x += child_w + fl->gap;
  } else {
    if (cursor_y + child_h > fl->container_y + fl->pad_top + avail_h) {
      cursor_y = fl->container_y + fl->pad_top;
      cursor_x += child_w + fl->gap;
    }
    *ox = cursor_x; *oy = cursor_y;
    *ow = (fl->align == CIV_ALIGN_STRETCH) ? avail_w - (cursor_x - fl->container_x - fl->pad_left) : child_w;
    *oh = child_h;
    cursor_y += child_h + fl->gap;
  }
}

int civ_flex_layout_row(civ_flex_layout_t *fl, float item_w, float item_h,
                        float **out_positions) {
  (void)fl; (void)item_w; (void)item_h; (void)out_positions;
  return 0; /* stub */
}

/* ── Grid layout ─────────────────────────────────────────────────────── */
void civ_grid_init(civ_grid_layout_t *gl, float x, float y, float w, float h,
                   int cols, int rows) {
  memset(gl, 0, sizeof(*gl));
  gl->container_x = x; gl->container_y = y;
  gl->container_w = w; gl->container_h = h;
  gl->columns = cols; gl->rows = rows;
  gl->gap = 8.0f;
}

void civ_grid_cell(civ_grid_layout_t *gl, int col, int row, int col_span,
                   int row_span, float *ox, float *oy, float *ow, float *oh) {
  float cell_w = (gl->container_w - (float)(gl->columns - 1) * gl->gap -
                  gl->pad_left - gl->pad_right) / (float)gl->columns;
  float cell_h = (gl->container_h - (float)(gl->rows - 1) * gl->gap -
                  gl->pad_top - gl->pad_bottom) / (float)gl->rows;

  *ox = gl->container_x + gl->pad_left + (float)col * (cell_w + gl->gap);
  *oy = gl->container_y + gl->pad_top + (float)row * (cell_h + gl->gap);
  *ow = cell_w * (float)col_span + gl->gap * (float)(col_span - 1);
  *oh = cell_h * (float)row_span + gl->gap * (float)(row_span - 1);
}
