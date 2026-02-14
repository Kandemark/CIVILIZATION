/**
 * @file ideology_system.h
 * @brief Emergent Ideology System
 *
 * Allows for the dynamic creation and evolution of ideologies based on
 * value spectrums rather than predefined types.
 */

#ifndef CIVILIZATION_IDEOLOGY_SYSTEM_H
#define CIVILIZATION_IDEOLOGY_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Ideology Value (Axis) */
typedef struct {
  char name[STRING_SHORT_LEN]; /* e.g., "Authority", "Tradition", "Markets" */
  civ_float_t value; /* -1.0 to 1.0 (e.g., -1.0=Anarchy, 1.0=Totalitarian) */
} civ_ideology_value_t;

/* Policy */
typedef struct {
  char name[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];
  civ_ideology_value_t *required_values; /* Values required to enact */
  size_t required_count;

  /* Effects could be added here (function pointers or data) */
} civ_policy_t;

/* Ideology */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN]; /* Dynamic name, e.g., "Neo-Traditionalism" */

  civ_ideology_value_t *values;
  size_t value_count;
  size_t value_capacity;

  civ_policy_t *policies;
  size_t policy_count;
  size_t policy_capacity;

  /* Evolution & Branching */
  char parent_ideology_id[STRING_SHORT_LEN];
  size_t sect_count;

  civ_float_t coherence;  /* How consistent the values are */
  civ_float_t radicalism; /* How extreme the values are */
} civ_ideology_t;

/* Ideology System */
typedef struct {
  civ_ideology_t *ideologies;
  size_t ideology_count;
  size_t ideology_capacity;
} civ_ideology_system_t;

/* Functions */
civ_ideology_system_t *civ_ideology_system_create(void);
void civ_ideology_system_destroy(civ_ideology_system_t *system);

civ_ideology_t *civ_ideology_create(const char *name);
void civ_ideology_destroy(civ_ideology_t *ideology);

/* Value Management */
civ_result_t civ_ideology_set_value(civ_ideology_t *ideology, const char *axis,
                                    civ_float_t value);
civ_float_t civ_ideology_get_value(const civ_ideology_t *ideology,
                                   const char *axis);

/* Evolution */
civ_result_t civ_ideology_evolve(civ_ideology_t *ideology, const char *axis,
                                 civ_float_t shift);
civ_ideology_t *civ_ideology_split(civ_ideology_system_t *system,
                                   const civ_ideology_t *parent,
                                   const char *name);
civ_result_t civ_ideology_drift(civ_ideology_t *ideology,
                                civ_float_t corruption, civ_float_t stability);

void civ_ideology_update_metrics(
    civ_ideology_t *ideology); /* Recalculate coherence/radicalism */

/* Comparison */
civ_float_t civ_ideology_distance(const civ_ideology_t *a,
                                  const civ_ideology_t *b);

#endif /* CIVILIZATION_IDEOLOGY_SYSTEM_H */
