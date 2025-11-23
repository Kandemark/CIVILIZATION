/**
 * @file settlement_manager.c
 * @brief Implementation of Dynamic Settlement Manager
 */

#include "../../../include/core/world/settlement_manager.h"
#include <math.h>
#include <stdio.h>


civ_settlement_manager_t *civ_settlement_manager_create(void) {
  civ_settlement_manager_t *manager =
      CIV_MALLOC(sizeof(civ_settlement_manager_t));
  if (manager) {
    manager->settlements = NULL;
    manager->settlement_count = 0;
    manager->settlement_capacity = 0;
    manager->min_distance = 10.0f; // Arbitrary unit distance
  }
  return manager;
}

void civ_settlement_manager_destroy(civ_settlement_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->settlements);
    CIV_FREE(manager);
  }
}

civ_result_t civ_settlement_manager_add(civ_settlement_manager_t *manager,
                                        civ_settlement_t *settlement) {
  if (!manager || !settlement)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->settlement_count >= manager->settlement_capacity) {
    size_t new_cap = manager->settlement_capacity == 0
                         ? 16
                         : manager->settlement_capacity * 2;
    civ_settlement_t *new_arr =
        CIV_REALLOC(manager->settlements, new_cap * sizeof(civ_settlement_t));
    if (!new_arr)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->settlements = new_arr;
    manager->settlement_capacity = new_cap;
  }

  manager->settlements[manager->settlement_count++] = *settlement;
  return (civ_result_t){CIV_OK, "Settlement added"};
}

civ_float_t civ_calculate_site_suitability(civ_float_t x, civ_float_t y) {
  // Placeholder: In real game, query terrain, resources, water access here.
  // For now, return random suitability
  return (civ_float_t)(rand() % 100) / 100.0f;
}

civ_result_t civ_attempt_settlement_spawn(civ_settlement_manager_t *manager,
                                          civ_float_t x, civ_float_t y) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "No manager"};

  // Check distance to existing
  for (size_t i = 0; i < manager->settlement_count; i++) {
    civ_float_t dx = manager->settlements[i].x - x;
    civ_float_t dy = manager->settlements[i].y - y;
    if (sqrt(dx * dx + dy * dy) < manager->min_distance) {
      return (civ_result_t){CIV_ERROR_INVALID_STATE, "Too close to existing"};
    }
  }

  civ_float_t suitability = civ_calculate_site_suitability(x, y);
  if (suitability > 0.7f) {
    civ_settlement_t new_town;
    memset(&new_town, 0, sizeof(civ_settlement_t));
    snprintf(new_town.id, STRING_SHORT_LEN, "settle_%ld", (long)time(NULL));
    snprintf(new_town.name, STRING_MEDIUM_LEN, "New Settlement %zu",
             manager->settlement_count + 1);
    new_town.tier = CIV_SETTLEMENT_HAMLET;
    new_town.x = x;
    new_town.y = y;
    new_town.population = 100; // Starting pop
    new_town.founded_date = time(NULL);
    new_town.attractiveness = suitability;

    return civ_settlement_manager_add(manager, &new_town);
  }

  return (civ_result_t){CIV_OK, "Not suitable"};
}

civ_result_t civ_settlement_manager_update(civ_settlement_manager_t *manager,
                                           civ_float_t time_delta) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null manager"};

  for (size_t i = 0; i < manager->settlement_count; i++) {
    civ_settlement_t *s = &manager->settlements[i];

    // Organic growth
    civ_float_t growth = s->population * 0.01f * s->attractiveness * time_delta;
    s->population += (int64_t)growth;

    // Tier upgrade
    if (s->tier == CIV_SETTLEMENT_HAMLET && s->population > 1000)
      s->tier = CIV_SETTLEMENT_VILLAGE;
    else if (s->tier == CIV_SETTLEMENT_VILLAGE && s->population > 5000)
      s->tier = CIV_SETTLEMENT_TOWN;
    else if (s->tier == CIV_SETTLEMENT_TOWN && s->population > 20000)
      s->tier = CIV_SETTLEMENT_CITY;
    else if (s->tier == CIV_SETTLEMENT_CITY && s->population > 100000)
      s->tier = CIV_SETTLEMENT_METROPOLIS;
  }

  return (civ_result_t){CIV_OK, "Updated"};
}
