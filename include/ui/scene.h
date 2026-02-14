#ifndef CIVILIZATION_SCENE_H
#define CIVILIZATION_SCENE_H

#include "../../include/core/game.h"
#include "../../include/engine/input.h"
#include "../../include/engine/renderer.h"

typedef struct {
  void (*init)(void);
  void (*update)(civ_game_t *game, civ_input_state_t *input);
  void (*render)(SDL_Renderer *renderer, int win_w, int win_h, civ_game_t *game,
                 civ_input_state_t *input);
  void (*destroy)(void);
  int next_scene_id; // -1 if no change
} civ_scene_t;

/* Scene IDs */
enum {
  SCENE_SPLASH,
  SCENE_PROFILE_SELECT,
  SCENE_PROFILE_CREATE,
  SCENE_MAIN_MENU,
  SCENE_SETUP, // World Gen
  SCENE_SPAWN_SELECT,
  SCENE_GAME,
  SCENE_GOV,
  SCENE_CITY,
  SCENE_MIL,
  SCENE_CULT,
  SCENE_DIP,
  SCENE_TECH,
  SCENE_COUNT
};

/* Global scene management */
void civ_scene_manager_init(void);
void civ_scene_manager_update(civ_game_t *game, civ_input_state_t *input);
void civ_scene_manager_render(SDL_Renderer *renderer, int win_w, int win_h,
                              civ_game_t *game, civ_input_state_t *input);
void civ_scene_manager_shutdown(void);
void civ_scene_manager_switch(int scene_id);

#endif /* CIVILIZATION_SCENE_H */
