/**
 * @file game.c
 * @brief Implementation of the main game controller
 */

#include "../../include/core/game.h"
#include "../../include/common.h"
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Forward declarations */
static void initialize_systems(civ_game_t *game,
                               const civ_game_config_t *config);
static void update_systems(civ_game_t *game, civ_float_t time_delta);

civ_game_t *civ_game_create(void) {
  civ_game_t *game = (civ_game_t *)CIV_MALLOC(sizeof(civ_game_t));
  if (!game) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate game");
    return NULL;
  }

  memset(game, 0, sizeof(civ_game_t));
  game->state = CIV_GAME_STATE_INITIALIZING;
  game->event_log_capacity = 1000;
  game->event_log = (civ_game_log_entry_t *)CIV_CALLOC(
      game->event_log_capacity, sizeof(civ_game_log_entry_t));

  if (!game->event_log) {
    CIV_FREE(game);
    return NULL;
  }

  return game;
}
