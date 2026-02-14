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
#include "../military/units.h"
#include "map_generator.h"
#include "territory.h"

/* Settlement Tier */
typedef enum {
  CIV_SETTLEMENT_HAMLET = 0,
  CIV_SETTLEMENT_VILLAGE,
  CIV_SETTLEMENT_TOWN,
  CIV_SETTLEMENT_CITY,
  CIV_SETTLEMENT_METROPOLIS,
  CIV_SETTLEMENT_CAPITAL
} civ_settlement_tier_t;

/* Demographic Breakdown */
typedef struct {
  int64_t race_pop[8];     /* Pop count per race ID (Ethnicity) */
  int64_t language_pop[8]; /* Pop count per language ID */
  int64_t faith_pop[8];    /* Pop count per faith ID */

  civ_float_t literacy; /* 0.0 to 1.0 */
  civ_float_t health_index;
} civ_demographic_stats_t;

/* Infrastructure State */
typedef struct {
  civ_float_t road_quality;   /* Connection speed */
  civ_float_t fortifications; /* Defense bonus */
  civ_float_t port_capacity;  /* Trade throughput */
  civ_float_t sanitation;     /* Growth bonus */
} civ_infrastructure_stats_t;

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

  civ_demographic_stats_t demographics;
  civ_infrastructure_stats_t infrastructure;

  char region_id[STRING_SHORT_LEN]; /* Subunit it belongs to */

  time_t founded_date;

  /* Production System */
  civ_unit_type_t production_type;
  civ_float_t production_progress;
  civ_float_t production_target;
  bool is_producing;

  /* Culture System */
  civ_float_t culture_yield;
  civ_float_t accumulated_culture;
  int32_t territory_radius;

  /* Sovereign Affairs (Phase 9/11) */
  civ_float_t loyalty; /* 0.0 to 1.0 */
  civ_float_t unrest;  /* 0.0 to 1.0 */

  /* Phase 11 Identity Markers (Primary) */
  int32_t primary_ethnicity;
  int32_t primary_language;
  int32_t primary_faith;
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

/* Forward declaration */
struct civ_government;

civ_result_t civ_settlement_manager_update(civ_settlement_manager_t *manager,
                                           civ_map_t *map,
                                           struct civ_government *gov,
                                           civ_float_t time_delta);

/* Formation Logic */
civ_float_t civ_calculate_site_suitability(
    civ_float_t x, civ_float_t y); /* Placeholder for complex logic */
civ_result_t civ_attempt_settlement_spawn(civ_settlement_manager_t *manager,
                                          civ_float_t x, civ_float_t y);

civ_result_t civ_settlement_manager_add(civ_settlement_manager_t *manager,
                                        civ_settlement_t *settlement);

#endif /* CIVILIZATION_SETTLEMENT_MANAGER_H */
