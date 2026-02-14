/**
 * @file unification_engine.h
 * @brief Civilizational Mergers and Shared Supranational Structures
 */

#ifndef CIVILIZATION_UNIFICATION_ENGINE_H
#define CIVILIZATION_UNIFICATION_ENGINE_H

#include "../../common.h"
#include "../../types.h"
#include "international_organizations.h"

/* Unification Stage */
typedef enum {
  CIV_UNIFY_MARKET_INTEGRATION = 0,
  CIV_UNIFY_CURRENCY_UNION,
  CIV_UNIFY_POLITICAL_FEDERATION,
  CIV_UNIFY_TOTAL_ABSORPTION
} civ_unification_stage_t;

/* Merge Scenario Results */
typedef struct {
  char new_national_name[STRING_MEDIUM_LEN];
  civ_unification_stage_t stage;
  civ_float_t stability_impact;
  bool success;
} civ_merge_result_t;

/* Functions */
civ_merge_result_t civ_unification_propose_merger(const char *nation_a,
                                                  const char *nation_b,
                                                  civ_unification_stage_t goal);
civ_result_t civ_unification_merge_economies(const char *nation_a,
                                             const char *nation_b);
civ_result_t civ_unification_create_shared_currency(const char *name,
                                                    const char *nation_a,
                                                    const char *nation_b);

#endif /* CIVILIZATION_UNIFICATION_ENGINE_H */
