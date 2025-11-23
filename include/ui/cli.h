/**
 * @file cli.h
 * @brief Command Line Interface for Civilization Game
 */

#ifndef CIVILIZATION_UI_CLI_H
#define CIVILIZATION_UI_CLI_H

#include "../core/game.h"

/* CLI Context */
typedef struct {
  civ_game_t *game;
  bool running;
} civ_cli_t;

/* Functions */
civ_cli_t *civ_cli_create(civ_game_t *game);
void civ_cli_destroy(civ_cli_t *cli);

void civ_cli_update(civ_cli_t *cli);
void civ_cli_render_status(civ_cli_t *cli);
void civ_cli_handle_input(civ_cli_t *cli);

#endif /* CIVILIZATION_UI_CLI_H */
