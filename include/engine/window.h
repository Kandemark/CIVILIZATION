/**
 * @file window.h
 * @brief SDL3 window management system
 */

#ifndef CIV_ENGINE_WINDOW_H
#define CIV_ENGINE_WINDOW_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Window handle structure (opaque)
 */
typedef struct civ_window civ_window_t;

/**
 * Window creation flags
 */
typedef enum {
  CIV_WINDOW_WINDOWED = 0,
  CIV_WINDOW_FULLSCREEN = 1,
  CIV_WINDOW_FULLSCREEN_DESKTOP = 2,
  CIV_WINDOW_RESIZABLE = 4,
  CIV_WINDOW_BORDERLESS = 8,
  CIV_WINDOW_MAXIMIZED = 16
} civ_window_flags_t;

/**
 * Create a new window with SDL3
 * @param title Window title
 * @param width Window width (0 for desktop width)
 * @param height Window height (0 for desktop height)
 * @param flags Window creation flags
 * @return Window handle or NULL on failure
 */
civ_window_t *civ_window_create(const char *title, int width, int height,
                                uint32_t flags);

/**
 * Destroy window and free resources
 * @param window Window to destroy
 */
void civ_window_destroy(civ_window_t *window);

/**
 * Get SDL renderer from window
 * @param window Window handle
 * @return SDL_Renderer pointer
 */
SDL_Renderer *civ_window_get_renderer(civ_window_t *window);

/**
 * Get SDL window from window handle
 * @param window Window handle
 * @return SDL_Window pointer
 */
SDL_Window *civ_window_get_sdl_window(civ_window_t *window);

/**
 * Clear window with solid color
 * @param window Window to clear
 * @param color RGB color (0xRRGGBB format)
 */
void civ_window_clear(civ_window_t *window, uint32_t color);

/**
 * Present rendered frame
 * @param window Window to present
 */
void civ_window_present(civ_window_t *window);

/**
 * Get current window size
 * @param window Window handle
 * @param w Output width
 * @param h Output height
 */
void civ_window_get_size(civ_window_t *window, int *w, int *h);

/**
 * Get current window drawable size (for high DPI)
 * @param window Window handle
 * @param w Output width
 * @param h Output height
 */
void civ_window_get_drawable_size(civ_window_t *window, int *w, int *h);

/**
 * Set window title
 * @param window Window handle
 * @param title New title
 */
void civ_window_set_title(civ_window_t *window, const char *title);

/**
 * Toggle fullscreen mode
 * @param window Window handle
 * @param fullscreen True for fullscreen, false for windowed
 */
void civ_window_set_fullscreen(civ_window_t *window, bool fullscreen);

#ifdef __cplusplus
}
#endif

#endif /* CIV_ENGINE_WINDOW_H */
