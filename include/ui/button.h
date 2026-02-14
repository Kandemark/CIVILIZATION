/**
 * @file button.h
 * @brief UI button widget
 */

#ifndef CIV_UI_BUTTON_H
#define CIV_UI_BUTTON_H

#include "../engine/font.h"
#include "../engine/input.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Button widget
 */
typedef struct {
  int id;              /**< Unique button ID */
  int x, y, w, h;      /**< Button rectangle */
  const char *label;   /**< Button text */
  bool highlighted;    /**< True if button should be highlighted */
  bool enabled;        /**< True if button is enabled */
  uint32_t color;      /**< Button color */
  uint32_t text_color; /**< Text color */

  /* Animation state */
  float hover_anim;  /**< 0.0 to 1.0 */
  float active_anim; /**< 0.0 to 1.0 */
} civ_button_t;

/**
 * Create a button
 * @param id Unique button ID
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param label Button label text
 * @return Button instance
 */
civ_button_t civ_button_create(int id, int x, int y, int w, int h,
                               const char *label);

/**
 * Render button and handle interaction
 * @param renderer SDL renderer
 * @param font Font for button text
 * @param button Button to render
 * @param input Input state
 * @return True if button was clicked this frame
 */
bool civ_button_render(SDL_Renderer *renderer, civ_font_t *font,
                       civ_button_t *button, civ_input_state_t *input);

/**
 * Set button position
 * @param button Button to modify
 * @param x New X position
 * @param y New Y position
 */
void civ_button_set_position(civ_button_t *button, int x, int y);

/**
 * Set button size
 * @param button Button to modify
 * @param w New width
 * @param h New height
 */
void civ_button_set_size(civ_button_t *button, int w, int h);

/**
 * Set button enabled state
 * @param button Button to modify
 * @param enabled True to enable, false to disable
 */
void civ_button_set_enabled(civ_button_t *button, bool enabled);

/**
 * Set button highlight state
 * @param button Button to modify
 * @param highlighted True to highlight
 */
void civ_button_set_highlighted(civ_button_t *button, bool highlighted);

/**
 * Check if mouse is over button
 * @param button Button to check
 * @param input Input state
 * @return True if mouse is over button
 */
bool civ_button_is_hovered(const civ_button_t *button,
                           const civ_input_state_t *input);

#ifdef __cplusplus
}
#endif

#endif /* CIV_UI_BUTTON_H */
