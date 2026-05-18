/**
 * @file widget_base.h
 * @brief Base widget struct and common widget infrastructure
 *
 * Widgets use a retained-mode system: each widget is a struct with a rect,
 * unique ID, and state. The input system's hot_id/active_id tracks focus.
 * Widgets register themselves each frame via civ_widget_begin/civ_widget_end.
 */
#ifndef CIV_UI_WIDGET_BASE_H
#define CIV_UI_WIDGET_BASE_H

#include "../../engine/input.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_WIDGET_ID_MAX 64

typedef enum {
  CIV_WIDGET_BUTTON = 0,
  CIV_WIDGET_LABEL,
  CIV_WIDGET_PANEL,
  CIV_WIDGET_PROGRESS_BAR,
  CIV_WIDGET_TOGGLE,
  CIV_WIDGET_SCROLL_AREA,
  CIV_WIDGET_TEXT_INPUT,
  CIV_WIDGET_COUNT
} civ_widget_type_t;

typedef enum {
  CIV_WIDGET_NORMAL = 0,
  CIV_WIDGET_HOVERED,
  CIV_WIDGET_PRESSED,
  CIV_WIDGET_DISABLED,
} civ_widget_state_t;

typedef struct civ_widget_base {
  civ_widget_type_t type;
  char id[CIV_WIDGET_ID_MAX];
  float x, y, w, h;
  bool visible;
  bool enabled;
  int z_index;
  civ_widget_state_t state;
  struct civ_widget_base *parent;
  struct civ_widget_base **children;
  int child_count;
  int child_capacity;
} civ_widget_base_t;

void civ_widget_init(civ_widget_base_t *widget, civ_widget_type_t type,
                     const char *id, float x, float y, float width,
                     float height);
void civ_widget_add_child(civ_widget_base_t *parent, civ_widget_base_t *child);
void civ_widget_destroy_children(civ_widget_base_t *w);

civ_widget_state_t civ_widget_compute_state(civ_widget_base_t *w,
                                            civ_input_state_t *input,
                                            bool can_activate);
bool civ_widget_is_hovered(const civ_widget_base_t *w, civ_input_state_t *input);

#ifdef __cplusplus
}
#endif
#endif
