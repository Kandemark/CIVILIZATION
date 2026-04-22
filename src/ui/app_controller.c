#include "ui/app_controller.h"
#include "display/theme.h"
#include "engine/font.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include "ui/nuklear_ui.h"
#include "utils/paths.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

civ_window_mgr_t *g_window_mgr = NULL;

civ_result_t civ_app_controller_init(civ_app_controller_t *app, int argc,
                                     char **argv) {
  (void)argc;
  (void)argv;

  if (!app)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null app controller"};

  memset(app, 0, sizeof(*app));

  /* Resolve asset base path from executable location */
  {
    char *base = SDL_GetBasePath();
    civ_path_init(base ? base : "./");
    if (base) SDL_free(base);
  }

  /* Initialize theme BEFORE any widgets or scenes use it */
  civ_theme_init_default();

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
      civ_window_create("Dominion", 1280, 720, CIV_WINDOW_RESIZABLE);
  if (!app->window) {
    civ_font_system_shutdown();
    SDL_Quit();
    return (civ_result_t){CIV_ERROR_INITIALIZATION_FAILED,
                          "Failed to create window"};
  }

  civ_input_init(&app->input);

  /* Initialize Nuklear UI on the SDL renderer */
  nk_ui_init(civ_window_get_sdl_window(app->window),
             civ_window_get_renderer(app->window));

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
  civ_window_mgr_init(&app->window_mgr);
  g_window_mgr = &app->window_mgr;
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
      nk_ui_handle_event(&event);

      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_F4 &&
            (event.key.mod & SDL_KMOD_ALT)) {
          app->running = false;
        }
        if (event.key.key == SDLK_F11) {
          static bool is_fullscreen = false;
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

    /* Set window size in input state so scenes can use it during update */
    app->input.win_w = win_w;
    app->input.win_h = win_h;
    app->input.global_dt = app->delta_time;

    civ_scene_manager_update(app->game, &app->input);
    civ_window_mgr_input(&app->window_mgr, &app->input);

    civ_window_clear(app->window, CIV_COLOR_BG_DARK);

    /* Nuklear begin — sets g_nk_ctx so scenes can render Nuklear UI */
    nk_ui_begin();

    civ_scene_manager_render(civ_window_get_renderer(app->window), win_w, win_h,
                             app->game, &app->input);
    civ_window_mgr_render(&app->window_mgr, civ_window_get_renderer(app->window),
                          NULL);

    /* Nuklear end — flushes all draw commands */
    nk_ui_end();

    civ_window_present(app->window);

    civ_input_end_frame(&app->input);
    SDL_Delay(16);
  }
}

void civ_app_controller_shutdown(civ_app_controller_t *app) {
  if (!app)
    return;

  civ_scene_manager_shutdown();
  nk_ui_shutdown();  /* free Nuklear resources before SDL cleans up */

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
