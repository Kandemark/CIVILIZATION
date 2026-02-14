/**
 * @file institution.h
 * @brief Modular Institutional Framework for Infinite Simulation
 */

#ifndef CIVILIZATION_INSTITUTION_H
#define CIVILIZATION_INSTITUTION_H

#include "../../common.h"
#include "../../types.h"

/* Institutional Focus Types (Bitmask) */
typedef enum {
  CIV_INSTITUTION_FOCUS_NONE = 0,
  CIV_INSTITUTION_FOCUS_RESEARCH = 1 << 0,
  CIV_INSTITUTION_FOCUS_PRODUCTION = 1 << 1,
  CIV_INSTITUTION_FOCUS_COMMERCE = 1 << 2,
  CIV_INSTITUTION_FOCUS_STABILITY = 1 << 3,
  CIV_INSTITUTION_FOCUS_MILITARY = 1 << 4,
  CIV_INSTITUTION_FOCUS_IDENTITY =
      1 << 5 /* Language/Religion/Culture influence */
} civ_institution_focus_t;

/* Institution Structure */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  uint32_t focuses; /* Bitmask of focus types */

  civ_float_t stature;          /* Infinite XP metric */
  civ_float_t growth_rate;      /* Driven by funding/efficiency */
  civ_float_t maintenance_cost; /* Scales with stature */

  char governing_role[STRING_SHORT_LEN]; /* Who controls this? e.g. "Parliament"
                                          */

  bool is_active;
  bool is_dissolving;
} civ_institution_t;

/* Manager */
typedef struct {
  civ_institution_t *items;
  size_t count;
  size_t capacity;
} civ_institution_manager_t;

/* Functions */
civ_institution_manager_t *civ_institution_manager_create(void);
void civ_institution_manager_destroy(civ_institution_manager_t *manager);

civ_result_t civ_institution_found(civ_institution_manager_t *manager,
                                   const char *name, uint32_t focuses,
                                   const char *governing_role);

void civ_institution_update(civ_institution_manager_t *manager,
                            civ_float_t budget_total,
                            civ_float_t gov_efficiency, civ_float_t time_delta);

civ_float_t
civ_institution_get_total_bonus(const civ_institution_manager_t *manager,
                                civ_institution_focus_t focus);

#endif /* CIVILIZATION_INSTITUTION_H */
