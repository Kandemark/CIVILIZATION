/**
 * @file map_view.c
 * @brief Implementation of map view system
 */
#include "../../../include/core/world/map_view.h"
#include "../../../include/common.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_map_view_manager_t *civ_map_view_manager_create(civ_map_t *base_map) {
  civ_map_view_manager_t *manager =
      (civ_map_view_manager_t *)CIV_MALLOC(sizeof(civ_map_view_manager_t));
  if (!manager) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate map view manager");
    return NULL;
  }

  civ_map_view_manager_init(manager, base_map);
  return manager;
}

void civ_map_view_manager_destroy(civ_map_view_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->view_count; i++) {
    CIV_FREE(manager->views[i].data);
  }
  CIV_FREE(manager->views);
  CIV_FREE(manager);
}

void civ_map_view_manager_init(civ_map_view_manager_t *manager,
                               civ_map_t *base_map) {
  if (!manager)
    return;

  memset(manager, 0, sizeof(civ_map_view_manager_t));
  manager->base_map = base_map;
  manager->view_count = CIV_MAP_VIEW_COUNT;
  manager->views =
      (civ_map_view_t *)CIV_CALLOC(manager->view_count, sizeof(civ_map_view_t));
  manager->current_view = CIV_MAP_VIEW_GEOGRAPHICAL;

  if (manager->views && base_map) {
    size_t tile_count = (size_t)base_map->width * (size_t)base_map->height;

    for (size_t i = 0; i < manager->view_count; i++) {
      manager->views[i].view_type = (civ_map_view_type_t)i;
      manager->views[i].width = base_map->width;
      manager->views[i].height = base_map->height;
      manager->views[i].visible = (i == CIV_MAP_VIEW_GEOGRAPHICAL);
      manager->views[i].opacity = 1.0f;
      manager->views[i].data =
          (civ_float_t *)CIV_CALLOC(tile_count, sizeof(civ_float_t));
    }
  }
}

civ_result_t civ_map_view_manager_set_view(civ_map_view_manager_t *manager,
                                           civ_map_view_type_t view_type) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (view_type >= CIV_MAP_VIEW_COUNT) {
    result.error = CIV_ERROR_INVALID_ARGUMENT;
    return result;
  }

  /* Hide current view */
  if (manager->current_view < CIV_MAP_VIEW_COUNT) {
    manager->views[manager->current_view].visible = false;
  }

  /* Show new view */
  manager->current_view = view_type;
  manager->views[view_type].visible = true;

  /* Update view data */
  civ_map_view_manager_update_view(manager, view_type);

  return result;
}

civ_result_t civ_map_view_manager_update_view(civ_map_view_manager_t *manager,
                                              civ_map_view_type_t view_type) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager || !manager->base_map) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (view_type >= CIV_MAP_VIEW_COUNT) {
    result.error = CIV_ERROR_INVALID_ARGUMENT;
    return result;
  }

  civ_map_view_t *view = &manager->views[view_type];

  switch (view_type) {
  case CIV_MAP_VIEW_GEOGRAPHICAL:
    /* Use elevation and terrain */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        civ_map_tile_t *tile = civ_map_get_tile(manager->base_map, x, y);
        if (tile) {
          view->data[y * view->width + x] = tile->elevation;
        }
      }
    }
    break;

  case CIV_MAP_VIEW_POLITICAL:
    /* Use border/territory data */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        civ_map_tile_t *tile = civ_map_get_tile(manager->base_map, x, y);
        if (tile) {
          view->data[y * view->width + x] = tile->political_influence;
        }
      }
    }
    break;

  case CIV_MAP_VIEW_DEMOGRAPHICAL:
    /* Use population density data */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        civ_map_tile_t *tile = civ_map_get_tile(manager->base_map, x, y);
        if (tile) {
          view->data[y * view->width + x] = tile->population_density;
        }
      }
    }
    break;

  case CIV_MAP_VIEW_CULTURAL:
    /* Use cultural influence data */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        civ_map_tile_t *tile = civ_map_get_tile(manager->base_map, x, y);
        if (tile) {
          view->data[y * view->width + x] = tile->cultural_influence;
        }
      }
    }
    break;

  case CIV_MAP_VIEW_ECONOMIC:
    /* Use resources */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        civ_map_tile_t *tile = civ_map_get_tile(manager->base_map, x, y);
        if (tile) {
          view->data[y * view->width + x] = tile->resources;
        }
      }
    }
    break;

  default:
    /* Placeholder for other views */
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        view->data[y * view->width + x] = 0.0f;
      }
    }
    break;
  }

  return result;
}

civ_float_t civ_map_view_get_tile_value(const civ_map_view_manager_t *manager,
                                        civ_map_view_type_t view_type,
                                        int32_t x, int32_t y) {
  if (!manager || view_type >= CIV_MAP_VIEW_COUNT)
    return 0.0f;
  if (x < 0 || y < 0 || x >= manager->views[view_type].width ||
      y >= manager->views[view_type].height)
    return 0.0f;

  return manager->views[view_type]
      .data[y * manager->views[view_type].width + x];
}

civ_result_t civ_map_view_manager_refresh_all(civ_map_view_manager_t *manager) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  for (size_t i = 0; i < manager->view_count; i++) {
    civ_map_view_manager_update_view(manager, (civ_map_view_type_t)i);
  }

  return result;
}

// Function to smoothly transition between map views
void civ_map_view_manager_transition_view(civ_map_view_manager_t *manager,
                                          civ_map_view_type_t new_view,
                                          float transition_speed) {
  if (!manager || new_view >= CIV_MAP_VIEW_COUNT)
    return;

  civ_map_view_t *current_view = &manager->views[manager->current_view];
  civ_map_view_t *target_view = &manager->views[new_view];

  // Start transition
  float opacity = 0.0f;
  while (opacity < 1.0f) {
    opacity += transition_speed;
    if (opacity > 1.0f)
      opacity = 1.0f;

    // Blend the two views based on opacity
    for (int32_t y = 0; y < manager->base_map->height; y++) {
      for (int32_t x = 0; x < manager->base_map->width; x++) {
        float current_value = current_view->data[y * current_view->width + x];
        float target_value = target_view->data[y * target_view->width + x];
        float blended_value =
            current_value * (1.0f - opacity) + target_value * opacity;

        // Update the current view data with the blended value
        current_view->data[y * current_view->width + x] = blended_value;
      }
    }

    // Render the blended view (placeholder for actual rendering logic)
    // civ_map_view_render(manager);
  }

  // Finalize transition
  manager->current_view = new_view;
  target_view->visible = true;
}

// Map interaction state
typedef struct {
  float zoom;
  int pan_x;
  int pan_y;
  int hovered_tile_x;
  int hovered_tile_y;
  bool tooltip_active;
} civ_map_interaction_t;

// Initialize map interaction state
void civ_map_interaction_init(civ_map_interaction_t *interaction) {
  if (!interaction)
    return;
  interaction->zoom = 1.0f;
  interaction->pan_x = 0;
  interaction->pan_y = 0;
  interaction->hovered_tile_x = -1;
  interaction->hovered_tile_y = -1;
  interaction->tooltip_active = false;
}

// Update zoom and pan
void civ_map_interaction_update_zoom_pan(civ_map_interaction_t *interaction,
                                         float zoom_delta, int pan_delta_x,
                                         int pan_delta_y) {
  if (!interaction)
    return;
  interaction->zoom += zoom_delta;
  if (interaction->zoom < 0.2f)
    interaction->zoom = 0.2f;
  if (interaction->zoom > 4.0f)
    interaction->zoom = 4.0f;
  interaction->pan_x += pan_delta_x;
  interaction->pan_y += pan_delta_y;
}

// Update hovered tile for tooltips
void civ_map_interaction_update_hover(civ_map_interaction_t *interaction,
                                      int mouse_x, int mouse_y, int tile_width,
                                      int tile_height) {
  if (!interaction)
    return;
  interaction->hovered_tile_x = mouse_x / tile_width;
  interaction->hovered_tile_y = mouse_y / tile_height;
  interaction->tooltip_active = true;
}

// Render tooltip for hovered tile
void civ_map_interaction_render_tooltip(
    SDL_Renderer *renderer, const civ_map_view_manager_t *manager,
    const civ_map_interaction_t *interaction, int tile_width, int tile_height) {
  if (!renderer || !manager || !interaction || !interaction->tooltip_active)
    return;
  int x = interaction->hovered_tile_x;
  int y = interaction->hovered_tile_y;
  if (x < 0 || y < 0 || x >= manager->base_map->width ||
      y >= manager->base_map->height)
    return;
  civ_float_t value =
      civ_map_view_get_tile_value(manager, manager->current_view, x, y);
  SDL_FRect tooltipRect = {(float)(x * tile_width + 5),
                           (float)(y * tile_height + 5), 120.0f, 30.0f};
  SDL_SetRenderDrawColor(renderer, 40, 40, 40, 220);
  SDL_RenderFillRect(renderer, &tooltipRect);
  // Placeholder: Render value as text (requires SDL_ttf)
}

// Animated transition state for map layers
typedef struct {
  civ_map_view_type_t from_view;
  civ_map_view_type_t to_view;
  float opacity;
  float duration;
  float elapsed;
  bool active;
} civ_map_transition_t;

// Initialize transition state
void civ_map_transition_init(civ_map_transition_t *transition) {
  if (!transition)
    return;
  transition->from_view = CIV_MAP_VIEW_GEOGRAPHICAL;
  transition->to_view = CIV_MAP_VIEW_GEOGRAPHICAL;
  transition->opacity = 0.0f;
  transition->duration = 1.0f;
  transition->elapsed = 0.0f;
  transition->active = false;
}

// Start a transition
void civ_map_transition_start(civ_map_transition_t *transition,
                              civ_map_view_type_t from, civ_map_view_type_t to,
                              float duration) {
  if (!transition)
    return;
  transition->from_view = from;
  transition->to_view = to;
  transition->opacity = 0.0f;
  transition->duration = duration;
  transition->elapsed = 0.0f;
  transition->active = true;
}

// Update transition (call per frame)
void civ_map_transition_update(civ_map_transition_t *transition,
                               float delta_time) {
  if (!transition || !transition->active)
    return;
  transition->elapsed += delta_time;
  transition->opacity = transition->elapsed / transition->duration;
  if (transition->opacity >= 1.0f) {
    transition->opacity = 1.0f;
    transition->active = false;
  }
}

// Render blended map view during transition
void civ_map_transition_render(const civ_map_view_manager_t *manager,
                               const civ_map_transition_t *transition,
                               SDL_Renderer *renderer, int tile_width,
                               int tile_height) {
  if (!manager || !transition || !transition->active)
    return;
  for (int32_t y = 0; y < manager->base_map->height; y++) {
    for (int32_t x = 0; x < manager->base_map->width; x++) {
      float from_value =
          civ_map_view_get_tile_value(manager, transition->from_view, x, y);
      float to_value =
          civ_map_view_get_tile_value(manager, transition->to_view, x, y);
      float blended = from_value * (1.0f - transition->opacity) +
                      to_value * transition->opacity;
      Uint8 color = (Uint8)(blended * 255);
      SDL_SetRenderDrawColor(renderer, color, color, color, 255);
      SDL_FRect tileRect = {(float)(x * tile_width), (float)(y * tile_height),
                            (float)tile_width, (float)tile_height};
      SDL_RenderFillRect(renderer, &tileRect);
    }
  }
}

// Layer customization and filtering state
typedef struct {
  bool show_political;
  bool show_demographical;
  bool show_cultural;
  bool show_economic;
  int selected_faction;
  int selected_resource_type;
} civ_map_layer_filter_t;

// Initialize layer filter state
void civ_map_layer_filter_init(civ_map_layer_filter_t *filter) {
  if (!filter)
    return;
  filter->show_political = true;
  filter->show_demographical = false;
  filter->show_cultural = false;
  filter->show_economic = false;
  filter->selected_faction = -1;
  filter->selected_resource_type = -1;
}

// Example: filter map tile rendering
bool civ_map_layer_filter_tile(const civ_map_layer_filter_t *filter,
                               civ_map_view_type_t view_type, int faction,
                               int resource_type) {
  if (!filter)
    return true;
  if (view_type == CIV_MAP_VIEW_POLITICAL && !filter->show_political)
    return false;
  if (view_type == CIV_MAP_VIEW_DEMOGRAPHICAL && !filter->show_demographical)
    return false;
  if (view_type == CIV_MAP_VIEW_CULTURAL && !filter->show_cultural)
    return false;
  if (view_type == CIV_MAP_VIEW_ECONOMIC && !filter->show_economic)
    return false;
  if (filter->selected_faction != -1 && faction != filter->selected_faction)
    return false;
  if (filter->selected_resource_type != -1 &&
      resource_type != filter->selected_resource_type)
    return false;
  return true;
}

// Highlight hovered and selected tiles
void civ_map_view_render_highlight(SDL_Renderer *renderer,
                                   const civ_map_view_manager_t *manager,
                                   const civ_map_interaction_t *interaction,
                                   int tile_width, int tile_height) {
  if (!renderer || !manager || !interaction)
    return;
  int x = interaction->hovered_tile_x;
  int y = interaction->hovered_tile_y;
  if (x < 0 || y < 0 || x >= manager->base_map->width ||
      y >= manager->base_map->height)
    return;
  SDL_FRect highlightRect = {(float)(x * tile_width), (float)(y * tile_height),
                             (float)tile_width, (float)tile_height};
  SDL_SetRenderDrawColor(renderer, 255, 215, 0, 180); // Gold highlight
  SDL_RenderFillRect(renderer, &highlightRect);
}

// Responsive tile size calculation
int civ_map_view_calc_tile_width(int window_width, int map_width) {
  return window_width / map_width;
}

int civ_map_view_calc_tile_height(int window_height, int map_height) {
  return window_height / map_height;
}

// Batch rendering for performance
void civ_map_view_render_batch(SDL_Renderer *renderer,
                               const civ_map_view_manager_t *manager,
                               civ_map_view_type_t view_type, int tile_width,
                               int tile_height) {
  if (!renderer || !manager)
    return;
  for (int32_t y = 0; y < manager->base_map->height; y++) {
    for (int32_t x = 0; x < manager->base_map->width; x++) {
      civ_float_t value = civ_map_view_get_tile_value(manager, view_type, x, y);
      Uint8 color = (Uint8)(value * 255);
      SDL_SetRenderDrawColor(renderer, color, color, color, 255);
      SDL_FRect tileRect = {(float)(x * tile_width), (float)(y * tile_height),
                            (float)tile_width, (float)tile_height};
      SDL_RenderFillRect(renderer, &tileRect);
    }
  }
}

// Integration example for main rendering loop
void civ_map_view_render_all(SDL_Renderer *renderer,
                             const civ_map_view_manager_t *manager,
                             const civ_map_interaction_t *interaction,
                             const civ_map_transition_t *transition,
                             const civ_map_layer_filter_t *filter,
                             int window_width, int window_height) {
  int tile_width =
      civ_map_view_calc_tile_width(window_width, manager->base_map->width);
  int tile_height =
      civ_map_view_calc_tile_height(window_height, manager->base_map->height);
  if (transition && transition->active) {
    civ_map_transition_render(manager, transition, renderer, tile_width,
                              tile_height);
  } else {
    civ_map_view_render_batch(renderer, manager, manager->current_view,
                              tile_width, tile_height);
  }
  civ_map_view_render_highlight(renderer, manager, interaction, tile_width,
                                tile_height);
  civ_map_interaction_render_tooltip(renderer, manager, interaction, tile_width,
                                     tile_height);
}
