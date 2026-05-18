/**
 * @file layer.h
 * @brief Ordered render layer system for the display engine
 */
#ifndef CIV_DISPLAY_LAYER_H
#define CIV_DISPLAY_LAYER_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CIV_LAYER_BACKGROUND = 0,
  CIV_LAYER_MAP_TERRAIN,
  CIV_LAYER_MAP_OVERLAY,
  CIV_LAYER_TERRITORY_BORDER,
  CIV_LAYER_SETTLEMENTS,
  CIV_LAYER_UNITS,
  CIV_LAYER_MINIMAP,
  CIV_LAYER_HUD_TOP,
  CIV_LAYER_HUD_BUTTONS,
  CIV_LAYER_SIDEBAR_LEFT,
  CIV_LAYER_PANEL_CENTER,
  CIV_LAYER_PANEL_DIPLOMACY,
  CIV_LAYER_TURN_BUTTON,
  CIV_LAYER_NOTIFICATIONS,
  CIV_LAYER_DEBUG_OVERLAY,
  CIV_LAYER_COUNT
} civ_layer_id_t;

typedef void (*civ_layer_render_fn)(SDL_Renderer *r, int w, int h, void *userdata);

typedef struct {
  civ_layer_id_t id;
  const char *name;
  bool enabled;
  civ_layer_render_fn render;
  void *userdata;
} civ_layer_t;

typedef struct {
  civ_layer_t layers[CIV_LAYER_COUNT];
} civ_layer_stack_t;

void civ_layer_stack_init(civ_layer_stack_t *stack);
void civ_layer_stack_render(civ_layer_stack_t *stack, SDL_Renderer *r, int w, int h);

#ifdef __cplusplus
}
#endif
#endif
