/**
 * @file cultural_assimilation.h
 * @brief Cultural assimilation and adoption tracking
 */

#ifndef CIVILIZATION_CULTURAL_ASSIMILATION_H
#define CIVILIZATION_CULTURAL_ASSIMILATION_H

#include "../../common.h"
#include "../../types.h"
#include "cultural_identity.h"

/* Assimilation type */
typedef enum {
  CIV_ASSIMILATION_VOLUNTARY = 0, /* Natural adoption */
  CIV_ASSIMILATION_FORCED,        /* Imposed by force */
  CIV_ASSIMILATION_MIXED          /* Combination */
} civ_assimilation_type_t;

/* Integration Stage */
typedef enum {
  CIV_INTEGRATION_DISSENT = 0, /* Open resistance */
  CIV_INTEGRATION_ACCEPTANCE,  /* Passive coexistence */
  CIV_INTEGRATION_ADOPTION,    /* Active borrowing */
  CIV_INTEGRATION_ABSORPTION,  /* Complete merger */
  CIV_INTEGRATION_UNIFICATION  /* Indistinguishable */
} civ_integration_stage_t;

/* Assimilation Tool */
typedef enum {
  CIV_TOOL_EDUCATION = 0,       /* State education system */
  CIV_TOOL_PROPAGANDA,          /* Media and messaging */
  CIV_TOOL_MIGRATION,           /* Population movement */
  CIV_TOOL_ECONOMIC_INCENTIVE,  /* Jobs/Trade benefits */
  CIV_TOOL_RELIGIOUS_SYNCRETISM /* Merging beliefs */
} civ_assimilation_tool_t;

/* Assimilation event */
typedef struct {
  char source_culture_id[STRING_SHORT_LEN];
  char target_culture_id[STRING_SHORT_LEN];
  char region_id[STRING_SHORT_LEN];

  civ_assimilation_type_t type;
  civ_integration_stage_t stage;

  civ_float_t progress;   /* 0.0 to 1.0 */
  civ_float_t rate;       /* Assimilation rate per time unit */
  civ_float_t resistance; /* 0.0 to 1.0 - how much target resists */

  civ_assimilation_tool_t active_tools[4];
  size_t tool_count;

  int32_t population_affected;
  civ_float_t adoption_level; /* 0.0 to 1.0 */

  time_t start_time;
  time_t last_update;
} civ_assimilation_event_t;

/* Assimilation tracker */
typedef struct {
  civ_assimilation_event_t *events;
  size_t event_count;
  size_t event_capacity;

  civ_float_t base_assimilation_rate;
  civ_float_t forced_assimilation_multiplier;
} civ_assimilation_tracker_t;

/* Function declarations */
civ_assimilation_tracker_t *civ_assimilation_tracker_create(void);
void civ_assimilation_tracker_destroy(civ_assimilation_tracker_t *tracker);
void civ_assimilation_tracker_init(civ_assimilation_tracker_t *tracker);

civ_result_t civ_assimilation_tracker_add_event(
    civ_assimilation_tracker_t *tracker, const char *source_id,
    const char *target_id, const char *region_id, civ_assimilation_type_t type);
civ_result_t civ_assimilation_tracker_update(
    civ_assimilation_tracker_t *tracker,
    civ_cultural_identity_manager_t *identity_manager, civ_float_t time_delta);
civ_assimilation_event_t *
civ_assimilation_tracker_get_event(const civ_assimilation_tracker_t *tracker,
                                   const char *source_id,
                                   const char *target_id);
civ_float_t civ_assimilation_calculate_rate(
    civ_assimilation_tracker_t *tracker, const civ_cultural_identity_t *source,
    const civ_cultural_identity_t *target, civ_assimilation_type_t type);

/* Policy Management */
civ_result_t civ_assimilation_apply_tool(civ_assimilation_event_t *event,
                                         civ_assimilation_tool_t tool);
civ_result_t civ_assimilation_update_stage(civ_assimilation_event_t *event);

#endif /* CIVILIZATION_CULTURAL_ASSIMILATION_H */
