#include "../../include/ui/scene.h"
#include <SDL3/SDL.h>
#include <stddef.h>

/* Scene implementations (extern) */
extern civ_scene_t scene_splash;
extern civ_scene_t scene_profile_select;
extern civ_scene_t scene_profile_create;
extern civ_scene_t scene_main_menu;
extern civ_scene_t scene_setup;
extern civ_scene_t scene_spawn_select;
extern civ_scene_t scene_game;

static civ_scene_t *scenes[SCENE_COUNT];
static int current_scene_id = SCENE_SPLASH;
static int next_scene_id = -1;

void civ_scene_manager_init(void) {
  scenes[SCENE_SPLASH] = &scene_splash;
  scenes[SCENE_PROFILE_SELECT] = &scene_profile_select;
  scenes[SCENE_PROFILE_CREATE] = &scene_profile_create;
  scenes[SCENE_MAIN_MENU] = &scene_main_menu;
  scenes[SCENE_SETUP] = &scene_setup;
  scenes[SCENE_SPAWN_SELECT] = &scene_spawn_select;
  scenes[SCENE_GAME] = &scene_game;

  // Other scenes use game scene for now or placeholders
  for (int i = SCENE_GAME + 1; i < SCENE_COUNT; i++) {
    scenes[i] = &scene_game;
  }

  if (scenes[current_scene_id] && scenes[current_scene_id]->init) {
    scenes[current_scene_id]->init();
  }
}

void civ_scene_manager_update(civ_game_t *game, civ_input_state_t *input) {
  if (next_scene_id != -1) {
    if (scenes[current_scene_id] && scenes[current_scene_id]->destroy) {
      scenes[current_scene_id]->destroy();
    }
    current_scene_id = next_scene_id;
    next_scene_id = -1;
    if (scenes[current_scene_id] && scenes[current_scene_id]->init) {
      scenes[current_scene_id]->init();
    }
  }

  if (scenes[current_scene_id] && scenes[current_scene_id]->update) {
    scenes[current_scene_id]->update(game, input);
  }

  /* Check for internal scene switch */
  if (scenes[current_scene_id] &&
      scenes[current_scene_id]->next_scene_id != -1) {
    civ_scene_manager_switch(scenes[current_scene_id]->next_scene_id);
    scenes[current_scene_id]->next_scene_id = -1; // Reset
  }
}

void civ_scene_manager_render(SDL_Renderer *renderer, int win_w, int win_h,
                              civ_game_t *game, civ_input_state_t *input) {
  if (scenes[current_scene_id] && scenes[current_scene_id]->render) {
    scenes[current_scene_id]->render(renderer, win_w, win_h, game, input);
  }
}

void civ_scene_manager_shutdown(void) {
  if (scenes[current_scene_id] && scenes[current_scene_id]->destroy) {
    scenes[current_scene_id]->destroy();
  }
}

void civ_scene_manager_switch(int scene_id) {
  if (scene_id >= 0 && scene_id < SCENE_COUNT) {
    next_scene_id = scene_id;
  }
}
