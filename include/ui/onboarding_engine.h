/**
 * @file onboarding_engine.h
 * @brief Story-based Game Initialization and Character Creation
 */

#ifndef CIVILIZATION_ONBOARDING_ENGINE_H
#define CIVILIZATION_ONBOARDING_ENGINE_H

#include "../common.h"
#include "../core/visuals/vexillology.h"
#include "../types.h"


/* Character Profile */
typedef struct {
  char name[STRING_MEDIUM_LEN];
  char origin_background[STRING_MEDIUM_LEN];
} civ_player_character_t;

/* World Configuration */
typedef struct {
  civ_float_t land_water_ratio; /* 0.0 (all water) to 1.0 (all land) */
  int32_t continent_count;
  int32_t world_size; /* Pixel dimension */
} civ_world_config_t;

/* Onboarding Context */
typedef struct {
  civ_player_character_t player;
  civ_world_config_t world_config;
  civ_flag_t player_flag;
  bool onboarding_complete;
} civ_onboarding_t;

/* Functions */
void civ_onboarding_init(civ_onboarding_t *onboarding);
void civ_onboarding_set_player(civ_onboarding_t *onboarding, const char *name);
void civ_onboarding_configure_world(civ_onboarding_t *onboarding,
                                    civ_float_t land_ratio, int continents);

/* Narrative Save/Load */
civ_result_t civ_onboarding_save_profile(const civ_onboarding_t *onboarding,
                                         const char *filepath);
civ_result_t civ_onboarding_load_profile(civ_onboarding_t *onboarding,
                                         const char *filepath);

#endif /* CIVILIZATION_ONBOARDING_ENGINE_H */
