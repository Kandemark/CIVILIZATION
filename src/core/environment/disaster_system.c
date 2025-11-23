/**
 * @file disaster_system.c
 * @brief Implementation of Disaster System
 */

#include "../../../include/core/environment/disaster_system.h"
#include <math.h>
#include <stdio.h>


civ_disaster_manager_t *
civ_disaster_manager_create(civ_geography_t *geography) {
  civ_disaster_manager_t *manager = CIV_MALLOC(sizeof(civ_disaster_manager_t));
  if (manager) {
    manager->active_disasters = NULL;
    manager->disaster_count = 0;
    manager->disaster_capacity = 0;
    manager->geography = geography;
  }
  return manager;
}

void civ_disaster_manager_destroy(civ_disaster_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->active_disasters);
    CIV_FREE(manager);
  }
}

civ_result_t civ_disaster_trigger(civ_disaster_manager_t *manager,
                                  civ_disaster_type_t type,
                                  civ_coordinate_t location,
                                  civ_float_t severity) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->disaster_count >= manager->disaster_capacity) {
    size_t new_cap =
        manager->disaster_capacity == 0 ? 4 : manager->disaster_capacity * 2;
    civ_disaster_t *new_arr = CIV_REALLOC(manager->active_disasters,
                                          new_cap * sizeof(civ_disaster_t));
    if (!new_arr)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->active_disasters = new_arr;
    manager->disaster_capacity = new_cap;
  }

  civ_disaster_t *d = &manager->active_disasters[manager->disaster_count++];
  snprintf(d->id, STRING_SHORT_LEN, "dis_%ld", (long)time(NULL));
  d->type = type;

  const char *type_names[] = {"Earthquake", "Flood",  "Drought", "Wildfire",
                              "Hurricane",  "Plague", "Volcano"};
  snprintf(d->name, STRING_MEDIUM_LEN, "%s at %.1f, %.1f", type_names[type],
           location.latitude, location.longitude);

  d->location = location;
  d->radius = 50.0f * severity; // Arbitrary radius scaling
  d->severity = severity;
  d->start_time = time(NULL);
  d->duration_hours = (int)(24 * 7 * severity); // Up to a week or more
  d->active = true;

  return (civ_result_t){CIV_OK, "Disaster triggered"};
}

void civ_disaster_update(civ_disaster_manager_t *manager,
                         civ_float_t time_delta) {
  if (!manager)
    return;

  time_t now = time(NULL);

  // Check for expiration
  for (size_t i = 0; i < manager->disaster_count; i++) {
    civ_disaster_t *d = &manager->active_disasters[i];
    if (!d->active)
      continue;

    double elapsed_hours = difftime(now, d->start_time) / 3600.0;
    if (elapsed_hours > d->duration_hours) {
      d->active = false;
    }
  }

  // Random spawn logic could go here (e.g., 0.01% chance per tick based on
  // geography)
  if (rand() % 10000 < 5) { // 0.05% chance
    civ_coordinate_t loc = {(civ_float_t)(rand() % 100),
                            (civ_float_t)(rand() % 100)};
    civ_disaster_trigger(manager, (civ_disaster_type_t)(rand() % 7), loc,
                         (civ_float_t)(rand() % 100) / 100.0f);
  }
}

civ_float_t civ_disaster_calculate_damage(const civ_disaster_t *disaster,
                                          civ_coordinate_t target_loc) {
  if (!disaster || !disaster->active)
    return 0.0f;

  civ_float_t dx = disaster->location.latitude - target_loc.latitude;
  civ_float_t dy = disaster->location.longitude - target_loc.longitude;
  civ_float_t dist = sqrt(dx * dx + dy * dy);

  if (dist > disaster->radius)
    return 0.0f;

  // Linear falloff
  return disaster->severity * (1.0f - (dist / disaster->radius));
}
