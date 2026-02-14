/**
 * @file combat.c
 * @brief Implementation of combat system
 */

#include "../../../include/core/military/combat.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TERRAIN_COUNT 6
#define WEATHER_COUNT 5

static const char *terrain_types[] = {"plains", "forest", "mountains",
                                      "urban",  "river",  "fortified"};
static const char *weather_types[] = {"clear", "rain", "fog", "snow", "storm"};

static civ_float_t get_terrain_modifier(const char *terrain) {
  if (!terrain)
    return 1.0f;

  if (strcmp(terrain, "plains") == 0)
    return 1.0f;
  if (strcmp(terrain, "forest") == 0)
    return 0.8f;
  if (strcmp(terrain, "mountains") == 0)
    return 0.6f;
  if (strcmp(terrain, "urban") == 0)
    return 0.7f;
  if (strcmp(terrain, "river") == 0)
    return 0.75f;
  if (strcmp(terrain, "fortified") == 0)
    return 0.5f;
  return 1.0f;
}

static civ_float_t get_weather_modifier(const char *weather) {
  if (!weather)
    return 1.0f;

  if (strcmp(weather, "clear") == 0)
    return 1.0f;
  if (strcmp(weather, "rain") == 0)
    return 0.9f;
  if (strcmp(weather, "fog") == 0)
    return 0.8f;
  if (strcmp(weather, "snow") == 0)
    return 0.7f;
  if (strcmp(weather, "storm") == 0)
    return 0.6f;
  return 1.0f;
}

civ_combat_system_t *civ_combat_system_create(void) {
  civ_combat_system_t *cs =
      (civ_combat_system_t *)CIV_MALLOC(sizeof(civ_combat_system_t));
  if (!cs) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate combat system");
    return NULL;
  }

  civ_combat_system_init(cs);
  return cs;
}

void civ_combat_system_destroy(civ_combat_system_t *cs) {
  if (!cs)
    return;
  CIV_FREE(cs->terrain_modifiers);
  CIV_FREE(cs->weather_modifiers);
  CIV_FREE(cs->combat_history);
  CIV_FREE(cs);
}

void civ_combat_system_init(civ_combat_system_t *cs) {
  if (!cs)
    return;

  memset(cs, 0, sizeof(civ_combat_system_t));

  cs->history_capacity = 100;
  cs->combat_history = (civ_combat_result_t *)CIV_CALLOC(
      cs->history_capacity, sizeof(civ_combat_result_t));
}

civ_float_t civ_combat_system_calculate_effectiveness(civ_combat_system_t *cs,
                                                      civ_float_t base_strength,
                                                      const char *terrain,
                                                      const char *weather) {
  if (!cs)
    return 0.0f;

  civ_float_t terrain_mod = get_terrain_modifier(terrain);
  civ_float_t weather_mod = get_weather_modifier(weather);

  return base_strength * terrain_mod * weather_mod;
}

civ_combat_result_t civ_combat_system_simulate_battle(
    civ_combat_system_t *cs, const char *attacker_nation,
    const char *defender_nation, const char *terrain_type) {
  civ_combat_result_t result = {0};

  if (!cs || !attacker_nation || !defender_nation)
    return result;

  /* Simplified battle simulation */
  /* In full implementation, would use actual unit data */
  civ_float_t attacker_strength = 1000.0f; /* Placeholder */
  civ_float_t defender_strength = 800.0f;  /* Placeholder */

  civ_float_t terrain_mod = get_terrain_modifier(terrain_type);
  attacker_strength *= terrain_mod;
  defender_strength *= terrain_mod * 1.1f; /* Defender advantage */

  /* Simulate combat rounds */
  int32_t rounds = 0;
  civ_float_t att_casualties = 0.0f;
  civ_float_t def_casualties = 0.0f;

  while (rounds < 20 && attacker_strength > 0 && defender_strength > 0) {
    rounds++;

    /* Damage exchange */
    civ_float_t att_damage = attacker_strength * 0.1f;
    civ_float_t def_damage = defender_strength * 0.08f;

    defender_strength = MAX(0.0f, defender_strength - att_damage);
    attacker_strength = MAX(0.0f, attacker_strength - def_damage);

    def_casualties += att_damage;
    att_casualties += def_damage;
  }

  /* Determine victor */
  bool attacker_wins = attacker_strength > defender_strength;
  strcpy(result.victor, attacker_wins ? attacker_nation : defender_nation);
  result.casualties_attacker = (int32_t)att_casualties;
  result.casualties_defender = (int32_t)def_casualties;
  result.prisoners = attacker_wins ? (int32_t)(def_casualties * 0.2f)
                                   : (int32_t)(att_casualties * 0.1f);
  result.territory_gained = attacker_wins;
  result.duration = rounds;

  /* Store in history */
  if (cs->history_count < cs->history_capacity) {
    cs->combat_history[cs->history_count++] = result;
  }

  return result;
}

civ_combat_result_t civ_combat_unit_vs_unit(civ_unit_t *attacker,
                                            civ_unit_t *defender,
                                            const char *terrain) {
  civ_combat_result_t result = {0};
  if (!attacker || !defender)
    return result;

  civ_float_t terrain_mod = get_terrain_modifier(terrain);

  /* Calculate effective strengths */
  civ_float_t a_eff =
      attacker->combat_strength * (attacker->morale + 0.5f) * terrain_mod;
  civ_float_t d_eff = defender->combat_strength * (defender->morale + 0.5f) *
                      terrain_mod * 1.2f; /* Def bonus */

  /* Damage is proportional to strength */
  int32_t a_damage = (int32_t)(d_eff * 0.2f * (rand() % 10 + 5) / 10.0f);
  int32_t d_damage = (int32_t)(a_eff * 0.2f * (rand() % 10 + 5) / 10.0f);

  /* Apply damage */
  attacker->current_strength = MAX(0, attacker->current_strength - a_damage);
  defender->current_strength = MAX(0, defender->current_strength - d_damage);

  /* Update morale */
  attacker->morale = MAX(0.1f, attacker->morale - 0.05f);
  defender->morale = MAX(0.1f, defender->morale - 0.05f);

  /* Award Experience */
  attacker->experience += d_damage * 0.5f;
  defender->experience += a_damage * 0.5f;

  /* Check for promotions */
  civ_unit_check_level_up(attacker);
  civ_unit_check_level_up(defender);

  /* Result */
  result.casualties_attacker = a_damage;
  result.casualties_defender = d_damage;
  strcpy(result.victor,
         (attacker->current_strength > defender->current_strength)
             ? "Attacker"
             : "Defender");

  return result;
}
