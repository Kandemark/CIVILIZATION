/**
 * @file font.h
 * @brief SDL3_ttf font rendering system
 */

#ifndef CIV_ENGINE_FONT_H
#define CIV_ENGINE_FONT_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Font handle (opaque)
 */
typedef struct civ_font civ_font_t;

/**
 * Text alignment
 */
typedef enum {
  CIV_ALIGN_LEFT,
  CIV_ALIGN_CENTER,
  CIV_ALIGN_RIGHT
} civ_text_align_t;

/**
 * Text vertical alignment
 */
typedef enum {
  CIV_VALIGN_TOP,
  CIV_VALIGN_MIDDLE,
  CIV_VALIGN_BOTTOM
} civ_text_valign_t;

/**
 * Initialize font system (call once at startup)
 * @return True on success
 */
bool civ_font_system_init(void);

/**
 * Shutdown font system (call once at shutdown)
 */
void civ_font_system_shutdown(void);

/**
 * Load system font by name
 * @param name Font name (e.g., "Segoe UI", "Arial", "Consolas")
 * @param size Font size in points
 * @return Font handle or NULL on failure
 */
civ_font_t *civ_font_load_system(const char *name, int size);

/**
 * Load font from file
 * @param path Path to TTF file
 * @param size Font size in points
 * @return Font handle or NULL on failure
 */
civ_font_t *civ_font_load(const char *path, int size);

/**
 * Destroy font and free resources
 * @param font Font to destroy
 */
void civ_font_destroy(civ_font_t *font);

/**
 * Render text at position
 * @param renderer SDL renderer
 * @param font Font to use
 * @param text Text to render
 * @param x X position
 * @param y Y position
 * @param color RGB color (0xRRGGBB)
 */
void civ_font_render(SDL_Renderer *renderer, civ_font_t *font, const char *text,
                     int x, int y, uint32_t color);

/**
 * Render text aligned within rectangle
 * @param renderer SDL renderer
 * @param font Font to use
 * @param text Text to render
 * @param x Rectangle X
 * @param y Rectangle Y
 * @param w Rectangle width
 * @param h Rectangle height
 * @param color RGB color (0xRRGGBB)
 * @param align Horizontal alignment
 * @param valign Vertical alignment
 */
void civ_font_render_aligned(SDL_Renderer *renderer, civ_font_t *font,
                             const char *text, int x, int y, int w, int h,
                             uint32_t color, civ_text_align_t align,
                             civ_text_valign_t valign);

/**
 * Render text with alpha blending
 * @param renderer SDL renderer
 * @param font Font to use
 * @param text Text to render
 * @param x X position
 * @param y Y position
 * @param color RGB color (0xRRGGBB)
 * @param alpha Alpha value (0-255)
 */
void civ_font_render_alpha(SDL_Renderer *renderer, civ_font_t *font,
                           const char *text, int x, int y, uint32_t color,
                           uint8_t alpha);

/**
 * Get text dimensions
 * @param font Font to measure with
 * @param text Text to measure
 * @param w Output width
 * @param h Output height
 */
void civ_font_get_text_size(civ_font_t *font, const char *text, int *w, int *h);

/**
 * Get font height (useful for line spacing)
 * @param font Font handle
 * @return Font height in pixels
 */
int civ_font_get_height(civ_font_t *font);

#ifdef __cplusplus
}
#endif

#endif /* CIV_ENGINE_FONT_H */
