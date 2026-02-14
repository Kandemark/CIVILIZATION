/**
 * @file cli.h
 * @brief Command Line Interface for Civilization Game
 */

#ifndef CIVILIZATION_UI_CLI_H
#define CIVILIZATION_UI_CLI_H

#include "../core/game.h"
#include "onboarding_engine.h"

/* UI State */
typedef enum {
  CIV_UI_ONBOARDING_CHAR = 0,
  CIV_UI_ONBOARDING_WORLD,
  CIV_UI_WORLD_SPAWN,
  CIV_UI_ELECTION_EVENT,
  CIV_UI_OPPOSITION_ROLE,
  CIV_UI_MAIN_GAME,
  CIV_UI_MENU_GOVERNANCE,
  CIV_UI_MENU_RELIGION,
  CIV_UI_MENU_VEXILLOLOGY,
  CIV_UI_MENU_CITIES,
  CIV_UI_EXIT
} civ_cli_state_t;

/* CLI Context */
typedef struct {
  civ_game_t *game;
  civ_cli_state_t state;
  civ_onboarding_t onboarding;
  bool running;
} civ_cli_t;

/* Functions */
civ_cli_t *civ_cli_create(civ_game_t *game);
void civ_cli_destroy(civ_cli_t *cli);

void civ_cli_update(civ_cli_t *cli);
void civ_cli_render_status(civ_cli_t *cli);
void civ_cli_handle_input(civ_cli_t *cli);

/* Specialized Screens */
void civ_cli_show_character_creation(civ_cli_t *cli);
void civ_cli_show_world_config(civ_cli_t *cli);
void civ_cli_show_governance_designer(civ_cli_t *cli);
void civ_cli_show_religion_creator(civ_cli_t *cli);
void civ_cli_show_vexillology_studio(civ_cli_t *cli);
void civ_cli_show_city_directory(civ_cli_t *cli);

#endif /* CIVILIZATION_UI_CLI_H */
