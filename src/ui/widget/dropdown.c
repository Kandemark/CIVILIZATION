#include "ui/widget/dropdown.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_dropdown_t *civ_dropdown_create(const char *id, float x, float y,
                                    float w, float h) {
  civ_dropdown_t *dd = calloc(1, sizeof(*dd));
  if (!dd) return NULL;
  civ_widget_init(&dd->base, CIV_WIDGET_DROPDOWN, id, x, y, w, h);
  dd->selected_index = -1;
  dd->is_open = false;
  dd->list_h = 200.0f;
  dd->color_bg = g_theme.bg_medium;
  dd->color_border = g_theme.primary;
  dd->color_item = 0x8899AA;
  dd->color_item_hover = 0xFFFFFF;
  return dd;
}

void civ_dropdown_destroy(civ_dropdown_t *dd) { free(dd); }

void civ_dropdown_add_item(civ_dropdown_t *dd, const char *text) {
  if (!dd || !text || dd->item_count >= CIV_DROPDOWN_MAX_ITEMS) return;
  strncpy(dd->items[dd->item_count], text, CIV_DROPDOWN_ITEM_LEN - 1);
  dd->item_count++;
}

void civ_dropdown_set_selected(civ_dropdown_t *dd, int index) {
  if (!dd || index < 0 || index >= dd->item_count) return;
  dd->selected_index = index;
}

int civ_dropdown_get_selected(const civ_dropdown_t *dd) {
  return dd ? dd->selected_index : -1;
}

const char *civ_dropdown_selected_text(const civ_dropdown_t *dd) {
  if (!dd || dd->selected_index < 0 || dd->selected_index >= dd->item_count)
    return "";
  return dd->items[dd->selected_index];
}

static bool dd_point_in(float px, float py, float rx, float ry, float rw, float rh) {
  return px >= rx && py >= ry && px <= rx + rw && py <= ry + rh;
}

void civ_dropdown_update(civ_dropdown_t *dd, civ_input_state_t *input, float dt) {
  if (!dd || !input) return;
  (void)dt;

  bool hover_main = civ_widget_is_hovered(&dd->base, input);

  /* Toggle open on click */
  if (hover_main && input->mouse_left_pressed) {
    dd->is_open = !dd->is_open;
  }

  /* Select item from open list */
  if (dd->is_open && input->mouse_left_pressed && !hover_main) {
    for (int i = 0; i < dd->item_count; i++) {
      float iy = dd->base.y + dd->base.h + (float)i * 24.0f;
      if (dd_point_in((float)input->mouse_x, (float)input->mouse_y,
                      dd->base.x, iy, dd->base.w, 24.0f)) {
        dd->selected_index = i;
        dd->is_open = false;
        break;
      }
    }
  }

  /* Close on click outside */
  if (dd->is_open && input->mouse_left_pressed && !hover_main) {
    float list_bottom = dd->base.y + dd->base.h +
        (float)dd->item_count * 24.0f;
    bool in_list = dd_point_in((float)input->mouse_x, (float)input->mouse_y,
        dd->base.x, dd->base.y + dd->base.h, dd->base.w,
        list_bottom - dd->base.y - dd->base.h);
    if (!in_list) dd->is_open = false;
  }
}

void civ_dropdown_render(civ_dropdown_t *dd, SDL_Renderer *r) {
  if (!dd || !r || !dd->base.visible) return;
  int x = (int)dd->base.x, y = (int)dd->base.y;
  int w = (int)dd->base.w, h = (int)dd->base.h;

  /* Main button */
  civ_render_rect_filled(r, x, y, w, h, dd->color_bg);
  civ_render_rect_outline(r, x, y, w, h, dd->color_border, 1);
  /* Arrow indicator */
  civ_render_rect_filled(r, x + w - 18, y + h/2 - 2, 8, 2, 0x8899AA);
  civ_render_rect_filled(r, x + w - 14, y + h/2 - 2, 6, 2, 0x8899AA);

  /* Selected text or placeholder */
  if (dd->selected_index >= 0) {
    /* Text handled by caller or label widget */
  }

  /* Dropdown list */
  if (dd->is_open) {
    for (int i = 0; i < dd->item_count; i++) {
      float iy = y + h + (float)i * 24.0f;
      bool hov = dd_point_in(0, 0, 0, 0, 0, 0); /* handled in update */
      uint32_t c = hov ? dd->color_item_hover : dd->color_item;
      if (i == dd->selected_index) c = dd->color_border;
      civ_render_rect_filled_alpha(r, x, (int)iy, w, 24, dd->color_bg, 240);
      civ_render_rect_outline(r, x, (int)iy, w, 24, dd->color_border, 1);
    }
  }
}
