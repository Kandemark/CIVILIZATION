/**
 * @file race_system.c
 * @brief Implementation of emergent biological races
 */

#include "../../../include/core/population/race_system.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_race_manager_t *civ_race_manager_create(void) {
  civ_race_manager_t *manager =
      (civ_race_manager_t *)CIV_MALLOC(sizeof(civ_race_manager_t));
  if (!manager)
    return NULL;

  manager->race_capacity = 16;
  manager->race_count = 0;
  manager->races =
      (civ_race_t *)CIV_CALLOC(manager->race_capacity, sizeof(civ_race_t));

  return manager;
}

void civ_race_manager_destroy(civ_race_manager_t *manager) {
  if (!manager)
    return;
  for (size_t i = 0; i < manager->race_count; i++) {
    CIV_FREE(manager->races[i].genome);
  }
  CIV_FREE(manager->races);
  CIV_FREE(manager);
}

civ_race_t *civ_race_emerge(civ_race_manager_t *manager, const char *name,
                            const char *region_id) {
  if (!manager || !name || !region_id)
    return NULL;

  if (manager->race_count >= manager->race_capacity) {
    manager->race_capacity *= 2;
    manager->races = (civ_race_t *)CIV_REALLOC(
        manager->races, manager->race_capacity * sizeof(civ_race_t));
  }

  if (manager->races) {
    civ_race_t *race = &manager->races[manager->race_count++];
    memset(race, 0, sizeof(civ_race_t));
    strncpy(race->id, name, STRING_SHORT_LEN - 1);
    strncpy(race->name, name, STRING_MEDIUM_LEN - 1);
    strncpy(race->origin_region_id, region_id, STRING_SHORT_LEN - 1);

    race->trait_capacity = 8;
    race->genome = (civ_dna_trait_t *)CIV_CALLOC(race->trait_capacity,
                                                 sizeof(civ_dna_trait_t));
    race->genetic_stability = 0.9f;
    race->emergence_time = time(NULL);

    return race;
  }

  return NULL;
}

civ_race_t *civ_race_merge(civ_race_manager_t *manager,
                           const civ_race_t *parent_a,
                           const civ_race_t *parent_b, const char *new_name) {
  if (!manager || !parent_a || !parent_b || !new_name)
    return NULL;

  civ_race_t *child =
      civ_race_emerge(manager, new_name, parent_a->origin_region_id);
  if (child) {
    /* Merge genomes: random expressivity from parents */
    for (size_t i = 0; i < parent_a->trait_count && i < child->trait_capacity;
         i++) {
      civ_dna_trait_t *trait = &child->genome[child->trait_count++];
      strncpy(trait->name, parent_a->genome[i].name, STRING_SHORT_LEN - 1);
      /* Expression is weighted average or mutation */
      trait->expression =
          (parent_a->genome[i].expression + parent_b->genome[i].expression) *
          0.5f;
      trait->dominant =
          parent_a->genome[i].dominant || parent_b->genome[i].dominant;
    }

    child->genetic_stability =
        (parent_a->genetic_stability + parent_b->genetic_stability) *
        0.45f; /* Loss of stability in hybrid */
    civ_log(CIV_LOG_INFO, "New race emerged from merging: %s", new_name);
  }

  return child;
}
