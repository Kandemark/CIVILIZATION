/**
 * @file renderer.h
 * @brief SDL3 rendering utilities
 */

#ifndef CIV_ENGINE_RENDERER_H
#define CIV_ENGINE_RENDERER_H

#include "../../include/core/world/map_generator.h"
#include "../../include/core/world/settlement_manager.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Draw filled rectangle
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color RGB color (0xRRGGBB)
 */
void civ_render_rect_filled(SDL_Renderer *renderer, int x, int y, int w, int h,
                            uint32_t color);

/**
 * Draw filled rectangle with alpha
 * @param alpha Alpha value (0-255)
 */
void civ_render_rect_filled_alpha(SDL_Renderer *renderer, int x, int y, int w,
                                  int h, uint32_t color, uint8_t alpha);

/**
 * Draw rectangle outline
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color RGB color (0xRRGGBB)
 * @param thickness Line thickness
 */
void civ_render_rect_outline(SDL_Renderer *renderer, int x, int y, int w, int h,
                             uint32_t color, int thickness);

/**
 * Draw rounded rectangle (filled)
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param radius Corner radius
 * @param color RGB color (0xRRGGBB)
 */
void civ_render_rounded_rect(SDL_Renderer *renderer, int x, int y, int w, int h,
                             int radius, uint32_t color);

/**
 * Draw line
 * @param renderer SDL renderer
 * @param x1 Start X
 * @param y1 Start Y
 * @param x2 End X
 * @param y2 End Y
 * @param color RGB color (0xRRGGBB)
 */
void civ_render_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2,
                     uint32_t color);

/**
 * Draw gradient background
 * @param renderer SDL renderer
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param color_top Top color (0xRRGGBB)
 * @param color_bottom Bottom color (0xRRGGBB)
 */
void civ_render_gradient_vertical(SDL_Renderer *renderer, int x, int y, int w,
                                  int h, uint32_t color_top,
                                  uint32_t color_bottom);

/**
 * Map rendering context
 */
typedef struct {
  SDL_Texture *map_texture; /**< Texture for map rendering */
  uint32_t *pixel_buffer;   /**< CPU-side pixel buffer */
  int buffer_width;         /**< Buffer width */
  int buffer_height;        /**< Buffer height */

  /* Camera state */
  float view_x; /**< Camera X position in world coords */
  float view_y; /**< Camera Y position in world coords */
  float zoom;   /**< Zoom level */

  /* Constants */
  int map_width;  /**< World map width */
  int map_height; /**< World map height */
} civ_render_map_context_t;

/**
 * Create map rendering context
 * @param renderer SDL renderer
 * @param fb_width Framebuffer width
 * @param fb_height Framebuffer height
 * @param map_width World map width
 * @param map_height World map height
 * @return Map context or NULL on failure
 */
civ_render_map_context_t *
civ_render_map_context_create(SDL_Renderer *renderer, int fb_width,
                              int fb_height, int map_width, int map_height);

/**
 * Destroy map rendering context
 * @param ctx Context to destroy
 */
void civ_render_map_context_destroy(civ_render_map_context_t *ctx);

/**
 * Render world map to screen
 * @param renderer SDL renderer
 * @param ctx Map rendering context
 * @param map World map data
 * @param fb_width Framebuffer width
 * @param fb_height Framebuffer height
 */
void civ_render_map(SDL_Renderer *renderer, civ_render_map_context_t *ctx,
                    civ_map_t *map, int fb_width, int fb_height);

/**
 * Render minimap to screen
 */
void civ_render_minimap(SDL_Renderer *renderer, int x, int y, int w, int h,
                        civ_map_t *map, civ_render_map_context_t *ctx);

/**
 * Render settlements on the map
 */
void civ_render_settlements(SDL_Renderer *renderer,
                            civ_render_map_context_t *ctx,
                            civ_settlement_manager_t *manager, int win_w,
                            int win_h);

/**
 * Set alpha blending mode
 * @param renderer SDL renderer
 * @param alpha Alpha value (0-255)
 */
void civ_render_set_alpha(SDL_Renderer *renderer, uint8_t alpha);

/**
 * Helper: Convert 0xRRGGBB to SDL_Color
 * @param color RGB color
 * @return SDL_Color with full alpha
 */
SDL_Color civ_color_from_rgb(uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* CIV_ENGINE_RENDERER_H */
