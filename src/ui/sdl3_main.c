/**
 * @file sdl3_main.c
 * @brief Main SDL3 application entry point for CIVILIZATION
 */

#include "../../include/core/game.h"
#include "../../include/core/world/map_generator.h"
#include "../../include/engine/font.h"
#include "../../include/engine/input.h"
#include "../../include/engine/renderer.h"
#include "../../include/engine/window.h"
#include "../../include/ui/button.h"
#include "../../include/ui/scene.h"
#include "../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Application state */
typedef struct {
  civ_window_t *window;
  civ_game_t *game;
  civ_input_state_t input;

  /* Timing */
  Uint64 last_frame_time;
  float delta_time;

} app_state_t;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  /* Initialize SDL3 */
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  printf("SDL3 initialized successfully\n");

  /* Initialize font system */
  if (!civ_font_system_init()) {
    SDL_Quit();
    return 1;
  }

  /* Create application state */
  app_state_t app = {0};

  /* Create window (0,0 = use desktop resolution) */
  app.window =
      civ_window_create("CIVILIZATION", 0, 0, CIV_WINDOW_FULLSCREEN_DESKTOP);
  if (!app.window) {
    civ_font_system_shutdown();
    SDL_Quit();
    return 1;
  }

  int win_w, win_h;
  civ_window_get_size(app.window, &win_w, &win_h);
  printf("Window size: %dx%d\n", win_w, win_h);

  /* Initialize input */
  civ_input_init(&app.input);

  /* Create game */
  app.game = civ_game_create();
  if (app.game) {
    civ_game_config_t config;
    civ_game_get_default_config(&config);
    civ_game_initialize(app.game, &config);
    /* civ_game_initialize already creates a default map, avoiding redundant
     * allocation */
  }

  /* Initialize Scene Manager */
  civ_scene_manager_init();

  /* Main loop */
  app.last_frame_time = SDL_GetTicksNS();
  bool running = true;
  SDL_Event event;

  while (running) {
    /* Calculate delta time */
    Uint64 current_time = SDL_GetTicksNS();
    app.delta_time =
        (float)(current_time - app.last_frame_time) / 1000000000.0f;
    app.last_frame_time = current_time;

    /* Begin input frame */
    civ_input_begin_frame(&app.input);

    /* Process events */
    while (SDL_PollEvent(&event)) {
      civ_input_process_event(&app.input, &event);

      /* Additional event handling */
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          running = false;
        }
        if (event.key.key == SDLK_F11) {
          /* Toggle fullscreen */
          static bool is_fullscreen = true;
          is_fullscreen = !is_fullscreen;
          civ_window_set_fullscreen(app.window, is_fullscreen);
        }
      }
    }

    if (app.input.quit_requested) {
      running = false;
    }

    /* Update Scene */
    civ_scene_manager_update(app.game, &app.input);

    /* Render */
    civ_window_clear(app.window, CIV_COLOR_BG_DARK);

    civ_scene_manager_render(civ_window_get_renderer(app.window), win_w, win_h,
                             app.game, &app.input);

    civ_window_present(app.window);

    /* End input frame */
    civ_input_end_frame(&app.input);

    /* Cap at ~60 FPS */
    SDL_Delay(16);
  }

  /* Cleanup */
  civ_scene_manager_shutdown();

  if (app.game) {
    civ_game_destroy(app.game);
  }

  civ_window_destroy(app.window);
  civ_font_system_shutdown();
  SDL_Quit();

  return 0;
}
