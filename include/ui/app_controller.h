#ifndef CIV_UI_APP_CONTROLLER_H
#define CIV_UI_APP_CONTROLLER_H

#include "../core/game.h"
#include "../engine/input.h"
#include "../engine/window.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  civ_window_t *window;
  civ_game_t *game;
  civ_input_state_t input;
  Uint64 last_frame_time;
  float delta_time;
  bool running;
} civ_app_controller_t;

civ_result_t civ_app_controller_init(civ_app_controller_t *app, int argc,
                                     char **argv);
void civ_app_controller_run(civ_app_controller_t *app);
void civ_app_controller_shutdown(civ_app_controller_t *app);

#ifdef __cplusplus
}
#endif

#endif /* CIV_UI_APP_CONTROLLER_H */
