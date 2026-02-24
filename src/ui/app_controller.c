#include "../../include/ui/app_controller.h"
#include "../../include/engine/font.h"
#include "../../include/ui/scene.h"
#include "../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

civ_result_t civ_app_controller_init(civ_app_controller_t *app, int argc,
                                     char **argv) {
  (void)argc;
  (void)argv;

  if (!app)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null app controller"};

  memset(app, 0, sizeof(*app));

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    return (civ_result_t){CIV_ERROR_INITIALIZATION_FAILED,
                          "Failed to initialize SDL"};
  }

  if (!civ_font_system_init()) {
    SDL_Quit();
    return (civ_result_t){CIV_ERROR_INITIALIZATION_FAILED,
                          "Failed to initialize font system"};
  }

  app->window =
      civ_window_create("CIVILIZATION", 0, 0, CIV_WINDOW_FULLSCREEN_DESKTOP);
  if (!app->window) {
    civ_font_system_shutdown();
    SDL_Quit();
    return (civ_result_t){CIV_ERROR_INITIALIZATION_FAILED,
                          "Failed to create window"};
  }

  civ_input_init(&app->input);

  app->game = civ_game_create();
  if (!app->game) {
    civ_app_controller_shutdown(app);
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "Failed to create game"};
  }

  civ_game_config_t config;
  civ_game_get_default_config(&config);
  civ_result_t init_result = civ_game_initialize(app->game, &config);
  if (CIV_FAILED(init_result)) {
    civ_app_controller_shutdown(app);
    return init_result;
  }

  civ_scene_manager_init();
  app->last_frame_time = SDL_GetTicksNS();
  app->running = true;

  return (civ_result_t){CIV_OK, NULL};
}

void civ_app_controller_run(civ_app_controller_t *app) {
  if (!app || !app->window)
    return;

  SDL_Event event;

  while (app->running) {
    Uint64 current_time = SDL_GetTicksNS();
    app->delta_time =
        (float)(current_time - app->last_frame_time) / 1000000000.0f;
    app->last_frame_time = current_time;

    civ_input_begin_frame(&app->input);

    while (SDL_PollEvent(&event)) {
      civ_input_process_event(&app->input, &event);

      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          app->running = false;
        }
        if (event.key.key == SDLK_F11) {
          static bool is_fullscreen = true;
          is_fullscreen = !is_fullscreen;
          civ_window_set_fullscreen(app->window, is_fullscreen);
        }
      }
    }

    if (app->input.quit_requested) {
      app->running = false;
    }

    int win_w = 0, win_h = 0;
    civ_window_get_size(app->window, &win_w, &win_h);

    civ_scene_manager_update(app->game, &app->input);

    civ_window_clear(app->window, CIV_COLOR_BG_DARK);
    civ_scene_manager_render(civ_window_get_renderer(app->window), win_w, win_h,
                             app->game, &app->input);
    civ_window_present(app->window);

    civ_input_end_frame(&app->input);
    SDL_Delay(16);
  }
}

void civ_app_controller_shutdown(civ_app_controller_t *app) {
  if (!app)
    return;

  civ_scene_manager_shutdown();

  if (app->game) {
    civ_game_destroy(app->game);
    app->game = NULL;
  }

  if (app->window) {
    civ_window_destroy(app->window);
    app->window = NULL;
  }

  civ_font_system_shutdown();
  SDL_Quit();
}
