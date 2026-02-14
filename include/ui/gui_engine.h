/**
 * @file gui_engine.h
 * @brief Premium Glassmorphic GUI and Layered Navigation
 */

#ifndef CIVILIZATION_GUI_ENGINE_H
#define CIVILIZATION_GUI_ENGINE_H

#include "../../include/render.h"
#include "../common.h"
#include "../types.h"


/* Interactive Window State */
typedef struct {
  char title[64];
  bool visible;
  civ_float_t opacity; /* Glassmorphism factor */
  int x, y, w, h;
} civ_gui_window_t;

/* UI Manager */
typedef struct {
  civ_gui_window_t windows[8];
  int active_window_count;
  bool show_hud;
  civ_render_viewport_t viewport;
} civ_gui_manager_t;

/* Functions */
void civ_gui_init(civ_gui_manager_t *ui);
void civ_gui_toggle_window(civ_gui_manager_t *ui, const char *title);
void civ_gui_render_frame(const civ_gui_manager_t *ui);

#endif /* CIVILIZATION_GUI_ENGINE_H */
