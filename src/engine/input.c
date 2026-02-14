/**
 * @file input.c
 * @brief SDL3 input handling implementation
 */

#include "../../include/engine/input.h"
#include <string.h>

void civ_input_init(civ_input_state_t *state) {
  if (!state)
    return;

  memset(state, 0, sizeof(civ_input_state_t));
  state->keyboard = SDL_GetKeyboardState(NULL);

  /* Initialize mouse pos to avoid jumping on first frame */
  float x, y;
  SDL_GetMouseState(&x, &y);
  state->last_mouse_x = (int)x;
  state->last_mouse_y = (int)y;
  state->mouse_x = (int)x;
  state->mouse_y = (int)y;
}

void civ_input_process_event(civ_input_state_t *state, const SDL_Event *event) {
  if (!state || !event)
    return;

  switch (event->type) {
  case SDL_EVENT_QUIT:
    state->quit_requested = true;
    break;

  case SDL_EVENT_MOUSE_MOTION:
    state->mouse_x = (int)event->motion.x;
    state->mouse_y = (int)event->motion.y;
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (event->button.button == SDL_BUTTON_LEFT) {
      state->mouse_left_down = true;
      state->mouse_left_pressed = true;
      /* Handle UI interaction */
      if (state->hot_id) {
        state->active_id = state->hot_id;
      }
    } else if (event->button.button == SDL_BUTTON_RIGHT) {
      state->mouse_right_down = true;
      state->mouse_right_pressed = true;
    }
    break;

  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (event->button.button == SDL_BUTTON_LEFT) {
      state->mouse_left_down = false;
      state->mouse_left_released = true;
      /* Handle button click */
      if (state->active_id && state->active_id == state->hot_id) {
        state->last_clicked_id = state->active_id;
      }
      state->active_id = 0;
    } else if (event->button.button == SDL_BUTTON_RIGHT) {
      state->mouse_right_down = false;
      state->mouse_right_released = true;
    }
    break;

  case SDL_EVENT_MOUSE_WHEEL:
    state->scroll_delta = event->wheel.y;
    break;

  case SDL_EVENT_TEXT_INPUT:
    strncpy(state->text_input, event->text.text, sizeof(state->text_input) - 1);
    state->text_input[sizeof(state->text_input) - 1] = '\0';
    break;

  case SDL_EVENT_KEY_DOWN:
    if (event->key.scancode == SDL_SCANCODE_BACKSPACE) {
      state->backspace_pressed = true;
    } else if (event->key.scancode == SDL_SCANCODE_RETURN) {
      state->enter_pressed = true;
    } else if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
      state->esc_pressed = true;
    }
    break;
  }
}

void civ_input_begin_frame(civ_input_state_t *state) {
  if (!state)
    return;

  /* Reset per-frame flags */
  state->mouse_left_pressed = false;
  state->mouse_left_released = false;
  state->mouse_right_pressed = false;
  state->mouse_right_released = false;
  state->scroll_delta = 0.0f;
  state->last_clicked_id = 0;

  /* Reset text input */
  state->text_input[0] = '\0';
  state->backspace_pressed = false;
  state->enter_pressed = false;
  state->esc_pressed = false;

  /* Reset deltas for the start of the frame processing */
  state->delta_x = 0;
  state->delta_y = 0;
}

void civ_input_end_frame(civ_input_state_t *state) {
  if (!state)
    return;

  /* Calculate deltas based on all events processed this frame */
  state->delta_x = state->mouse_x - state->last_mouse_x;
  state->delta_y = state->mouse_y - state->last_mouse_y;

  /* Store previous frame mouse position */
  state->last_mouse_x = state->mouse_x;
  state->last_mouse_y = state->mouse_y;
}

bool civ_input_is_mouse_over(const civ_input_state_t *state, int x, int y,
                             int w, int h) {
  if (!state)
    return false;

  return (state->mouse_x >= x && state->mouse_x <= x + w &&
          state->mouse_y >= y && state->mouse_y <= y + h);
}

bool civ_input_is_key_down(const civ_input_state_t *state,
                           SDL_Scancode scancode) {
  if (!state || !state->keyboard)
    return false;
  return state->keyboard[scancode] != 0;
}
