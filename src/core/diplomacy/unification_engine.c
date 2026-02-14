/**
 * @file unification_engine.c
 * @brief Implementation of civilizational mergers
 */

#include "../../../include/core/diplomacy/unification_engine.h"
#include "../../../include/common.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


civ_merge_result_t
civ_unification_propose_merger(const char *nation_a, const char *nation_b,
                               civ_unification_stage_t goal) {
  civ_merge_result_t result;
  memset(&result, 0, sizeof(civ_merge_result_t));

  /* Simple logic: Success if both names are valid */
  if (nation_a && nation_b) {
    result.success = true;
    result.stage = goal;
    snprintf(result.new_national_name, STRING_MEDIUM_LEN - 1,
             "The United Commonwealth of %s-%s", nation_a, nation_b);
    result.stability_impact = 0.1f; /* Small boost for unity */
  } else {
    result.success = false;
  }

  civ_log(CIV_LOG_INFO, "Merger proposal between %s and %s: %s", nation_a,
          nation_b, result.success ? "ACCEPTED" : "REJECTED");
  return result;
}

civ_result_t civ_unification_merge_economies(const char *nation_a,
                                             const char *nation_b) {
  civ_log(CIV_LOG_INFO,
          "Merging markets of %s and %s into a single trade bloc.", nation_a,
          nation_b);
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_unification_create_shared_currency(const char *name,
                                                    const char *nation_a,
                                                    const char *nation_b) {
  civ_log(CIV_LOG_INFO,
          "New shared currency '%s' established between %s and %s.", name,
          nation_a, nation_b);
  return (civ_result_t){CIV_OK, NULL};
}
