/**
 * @file race_system.h
 * @brief Emergent Biological Race and DNA system
 */

#ifndef CIVILIZATION_RACE_SYSTEM_H
#define CIVILIZATION_RACE_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Biological Trait (DNA) */
typedef struct {
  char name[STRING_SHORT_LEN];
  civ_float_t expression; /* 0.0 to 1.0 */
  bool dominant;
} civ_dna_trait_t;

/* Race Unit */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char origin_region_id[STRING_SHORT_LEN];

  civ_dna_trait_t *genome;
  size_t trait_count;
  size_t trait_capacity;

  civ_float_t genetic_stability; /* Higher = less likely to hybridize */
  time_t emergence_time;
} civ_race_t;

/* Race Manager */
typedef struct {
  civ_race_t *races;
  size_t race_count;
  size_t race_capacity;
} civ_race_manager_t;

/* Functions */
civ_race_manager_t *civ_race_manager_create(void);
void civ_race_manager_destroy(civ_race_manager_t *manager);

civ_race_t *civ_race_emerge(civ_race_manager_t *manager, const char *name,
                            const char *region_id);
civ_race_t *civ_race_merge(civ_race_manager_t *manager,
                           const civ_race_t *parent_a,
                           const civ_race_t *parent_b, const char *new_name);

#endif /* CIVILIZATION_RACE_SYSTEM_H */
