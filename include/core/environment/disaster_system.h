/**
 * @file disaster_system.h
 * @brief Environmental Disasters and Events
 */

#ifndef CIVILIZATION_DISASTER_SYSTEM_H
#define CIVILIZATION_DISASTER_SYSTEM_H

#include "../../common.h"
#include "../../types.h"
#include "../environment/geography.h"

/* Disaster Type */
typedef enum {
  CIV_DISASTER_EARTHQUAKE = 0,
  CIV_DISASTER_FLOOD,
  CIV_DISASTER_DROUGHT,
  CIV_DISASTER_WILDFIRE,
  CIV_DISASTER_HURRICANE,
  CIV_DISASTER_PLAGUE,
  CIV_DISASTER_VOLCANO
} civ_disaster_type_t;

/* Disaster Event */
typedef struct {
  char id[STRING_SHORT_LEN];
  civ_disaster_type_t type;
  char name[STRING_MEDIUM_LEN];

  civ_coordinate_t location;
  civ_float_t radius;   /* Area of effect */
  civ_float_t severity; /* 0.0 to 1.0 */

  time_t start_time;
  int32_t duration_hours;

  bool active;
} civ_disaster_t;

/* Disaster Manager */
typedef struct {
  civ_disaster_t *active_disasters;
  size_t disaster_count;
  size_t disaster_capacity;

  civ_geography_t *geography; /* Reference to world geography */
} civ_disaster_manager_t;

/* Functions */
civ_disaster_manager_t *civ_disaster_manager_create(civ_geography_t *geography);
void civ_disaster_manager_destroy(civ_disaster_manager_t *manager);

civ_result_t civ_disaster_trigger(civ_disaster_manager_t *manager,
                                  civ_disaster_type_t type,
                                  civ_coordinate_t location,
                                  civ_float_t severity);
void civ_disaster_update(civ_disaster_manager_t *manager,
                         civ_float_t time_delta);

/* Impact Calculation */
civ_float_t civ_disaster_calculate_damage(const civ_disaster_t *disaster,
                                          civ_coordinate_t target_loc);

#endif /* CIVILIZATION_DISASTER_SYSTEM_H */
