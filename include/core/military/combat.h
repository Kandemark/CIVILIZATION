/**
 * @file combat.h
 * @brief Combat system
 */

#ifndef CIVILIZATION_COMBAT_H
#define CIVILIZATION_COMBAT_H

#include "../../common.h"
#include "../../types.h"
#include "units.h"

/* Combat phase enumeration */
typedef enum {
  CIV_COMBAT_PHASE_PREP = 0,
  CIV_COMBAT_PHASE_RANGED,
  CIV_COMBAT_PHASE_MELEE,
  CIV_COMBAT_PHASE_PURSUIT,
  CIV_COMBAT_PHASE_AFTERMATH
} civ_combat_phase_t;

/* Combat result structure */
typedef struct {
  char victor[STRING_SHORT_LEN];
  int32_t casualties_attacker;
  int32_t casualties_defender;
  int32_t prisoners;
  bool territory_gained;
  int32_t duration;
} civ_combat_result_t;

/* Combat system structure */
typedef struct {
  civ_float_t *terrain_modifiers;
  civ_float_t *weather_modifiers;
  civ_combat_result_t *combat_history;
  size_t history_count;
  size_t history_capacity;
} civ_combat_system_t;

/* Function declarations */
civ_combat_system_t *civ_combat_system_create(void);
void civ_combat_system_destroy(civ_combat_system_t *cs);
void civ_combat_system_init(civ_combat_system_t *cs);

civ_combat_result_t civ_combat_system_simulate_battle(
    civ_combat_system_t *cs, const char *attacker_nation,
    const char *defender_nation, const char *terrain_type);

civ_float_t civ_combat_system_calculate_effectiveness(civ_combat_system_t *cs,
                                                      civ_float_t base_strength,
                                                      const char *terrain,
                                                      const char *weather);

/**
 * Simulate a direct skirmish between two units
 */
civ_combat_result_t civ_combat_unit_vs_unit(civ_unit_t *attacker,
                                            civ_unit_t *defender,
                                            const char *terrain);

#endif /* CIVILIZATION_COMBAT_H */
