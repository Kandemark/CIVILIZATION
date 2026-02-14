/**
 * @file input.h
 * @brief SDL3 input handling system
 */

#ifndef CIV_ENGINE_INPUT_H
#define CIV_ENGINE_INPUT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Input state for current frame
 */
typedef struct {
  /* Mouse state */
  int mouse_x;      /**< Current mouse X position */
  int mouse_y;      /**< Current mouse Y position */
  int last_mouse_x; /**< Previous frame mouse X */
  int last_mouse_y; /**< Previous frame mouse Y */
  int delta_x;      /**< Mouse delta X this frame */
  int delta_y;      /**< Mouse delta Y this frame */

  bool mouse_left_down;      /**< Left button held */
  bool mouse_right_down;     /**< Right button held */
  bool mouse_left_pressed;   /**< Left button just pressed this frame */
  bool mouse_left_released;  /**< Left button just released this frame */
  bool mouse_right_pressed;  /**< Right button just pressed this frame */
  bool mouse_right_released; /**< Right button just released this frame */

  float scroll_delta; /**< Mouse wheel delta */

  /* Keyboard state */
  const bool *keyboard; /**< Keyboard state array (SDL_GetKeyboardState) */

  /* Control flags */
  bool quit_requested; /**< True if quit event received */

  /* UI state */
  int hot_id;          /**< ID of widget under mouse */
  int active_id;       /**< ID of active/clicked widget */
  int last_clicked_id; /**< ID of widget clicked this frame */

  /* Text input and keys */
  char text_input[32];    /**< Text input received this frame (UTF-8) */
  bool backspace_pressed; /**< True if backspace was pressed this frame */
  bool enter_pressed;     /**< True if enter was pressed this frame */
  bool esc_pressed;       /**< True if escape was pressed this frame */
} civ_input_state_t;

/**
 * Initialize input state
 * @param state Input state to initialize
 */
void civ_input_init(civ_input_state_t *state);

/**
 * Process SDL event and update input state
 * @param state Input state to update
 * @param event SDL event to process
 */
void civ_input_process_event(civ_input_state_t *state, const SDL_Event *event);

/**
 * Begin new input frame (call at start of frame)
 * Reset per-frame flags like pressed/released
 * @param state Input state to reset
 */
void civ_input_begin_frame(civ_input_state_t *state);

/**
 * End input frame (call at end of frame)
 * Update deltas and previous state
 * @param state Input state to finalize
 */
void civ_input_end_frame(civ_input_state_t *state);

/**
 * Check if mouse is over rectangle
 * @param state Input state
 * @param x Rectangle X
 * @param y Rectangle Y
 * @param w Rectangle width
 * @param h Rectangle height
 * @return True if mouse is inside rectangle
 */
bool civ_input_is_mouse_over(const civ_input_state_t *state, int x, int y,
                             int w, int h);

/**
 * Check if key is currently down
 * @param state Input state
 * @param scancode SDL scancode
 * @return True if key is down
 */
bool civ_input_is_key_down(const civ_input_state_t *state,
                           SDL_Scancode scancode);

#ifdef __cplusplus
}
#endif

#endif /* CIV_ENGINE_INPUT_H */
