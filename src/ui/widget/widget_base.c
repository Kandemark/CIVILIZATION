#include "ui/widget/widget_base.h"
#include <stdlib.h>
#include <string.h>

void civ_widget_init(civ_widget_base_t *widget, civ_widget_type_t type,
                     const char *id, float x, float y, float width,
                     float height) {
  memset(widget, 0, sizeof(*widget));
  widget->type = type;
  strncpy(widget->id, id ? id : "widget", CIV_WIDGET_ID_MAX - 1);
  widget->x = x;
  widget->y = y;
  widget->w = width;
  widget->h = height;
  widget->visible = true;
  widget->enabled = true;
  widget->state = CIV_WIDGET_NORMAL;
}

void civ_widget_add_child(civ_widget_base_t *parent, civ_widget_base_t *child) {
  if (!parent || !child) return;
  if (parent->child_count >= parent->child_capacity) {
    int new_cap = parent->child_capacity ? parent->child_capacity * 2 : 4;
    civ_widget_base_t **new_children = (civ_widget_base_t **)realloc(
        parent->children, (size_t)new_cap * sizeof(civ_widget_base_t *));
    if (!new_children) return;
    parent->children = new_children;
    parent->child_capacity = new_cap;
  }
  parent->children[parent->child_count++] = child;
  child->parent = parent;
}

void civ_widget_destroy_children(civ_widget_base_t *w) {
  if (!w) return;
  for (int i = 0; i < w->child_count; i++) {
    civ_widget_destroy_children(w->children[i]);
    free(w->children[i]);
  }
  free(w->children);
  w->children = NULL;
  w->child_count = 0;
  w->child_capacity = 0;
}

bool civ_widget_is_hovered(const civ_widget_base_t *w, civ_input_state_t *input) {
  if (!w || !w->visible || !input) return false;
  return input->mouse_x >= (int)w->x && input->mouse_x <= (int)(w->x + w->w) &&
         input->mouse_y >= (int)w->y && input->mouse_y <= (int)(w->y + w->h);
}

civ_widget_state_t civ_widget_compute_state(civ_widget_base_t *w,
                                            civ_input_state_t *input,
                                            bool can_activate) {
  if (!w || !w->enabled) return CIV_WIDGET_DISABLED;
  if (!civ_widget_is_hovered(w, input)) return CIV_WIDGET_NORMAL;
  if (can_activate && input->mouse_left_down) return CIV_WIDGET_PRESSED;
  return CIV_WIDGET_HOVERED;
}
