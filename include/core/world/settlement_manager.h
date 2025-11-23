/**
 * @file settlement_manager.h
 * @brief Dynamic Settlement Formation System
 *
 * Handles the organic formation and growth of settlements based on
 * demographics, resources, and geographic suitability.
 */

#ifndef CIVILIZATION_SETTLEMENT_MANAGER_H
#define CIVILIZATION_SETTLEMENT_MANAGER_H

#include "../../common.h"
#include "../../types.h"
#include "territory.h"

/* Settlement Tier */
typedef enum {
  CIV_SETTLEMENT_HAMLET = 0,
  CIV_SETTLEMENT_VILLAGE,
  CIV_SETTLEMENT_TOWN,
  CIV_SETTLEMENT_CITY,
  CIV_SETTLEMENT_METROPOLIS
} civ_settlement_tier_t;

/* Settlement */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_settlement_tier_t tier;

  civ_float_t x;
  civ_float_t y;

  int64_t population;
  civ_float_t growth_rate;
  civ_float_t attractiveness; /* 0.0 to 1.0 */

  char region_id[STRING_SHORT_LEN]; /* Subunit it belongs to */

  time_t founded_date;
} civ_settlement_t;

/* Manager */
typedef struct {
  civ_settlement_t *settlements;
  size_t settlement_count;
  size_t settlement_capacity;

  civ_float_t min_distance; /* Min distance between settlements */
} civ_settlement_manager_t;

/* Functions */
civ_settlement_manager_t *civ_settlement_manager_create(void);
void civ_settlement_manager_destroy(civ_settlement_manager_t *manager);

civ_result_t civ_settlement_manager_update(civ_settlement_manager_t *manager,
                                           civ_float_t time_delta);

/* Formation Logic */
civ_float_t civ_calculate_site_suitability(
    civ_float_t x, civ_float_t y); /* Placeholder for complex logic */
civ_result_t civ_attempt_settlement_spawn(civ_settlement_manager_t *manager,
                                          civ_float_t x, civ_float_t y);

civ_result_t civ_settlement_manager_add(civ_settlement_manager_t *manager,
                                        civ_settlement_t *settlement);

#endif /* CIVILIZATION_SETTLEMENT_MANAGER_H */
