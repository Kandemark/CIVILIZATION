/**
 * @file ministry.c
 * @brief Implementation of ministry system
 */

#include "../../../include/core/governance/ministry.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_ministry_manager_t *civ_ministry_manager_create(void) {
  civ_ministry_manager_t *manager =
      (civ_ministry_manager_t *)CIV_MALLOC(sizeof(civ_ministry_manager_t));
  if (!manager)
    return NULL;

  memset(manager, 0, sizeof(civ_ministry_manager_t));
  manager->ministry_capacity = 8;
  manager->ministries = (civ_ministry_t *)CIV_CALLOC(manager->ministry_capacity,
                                                     sizeof(civ_ministry_t));

  return manager;
}

void civ_ministry_manager_destroy(civ_ministry_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->ministry_count; i++) {
    CIV_FREE(manager->ministries[i].active_proposals);
  }
  CIV_FREE(manager->ministries);
  CIV_FREE(manager);
}

civ_ministry_t *civ_ministry_create(civ_ministry_type_t type,
                                    const char *minister_name) {
  civ_ministry_t *ministry =
      (civ_ministry_t *)CIV_MALLOC(sizeof(civ_ministry_t));
  if (!ministry)
    return NULL;

  memset(ministry, 0, sizeof(civ_ministry_t));
  ministry->type = type;
  ministry->efficiency = 0.5f;
  ministry->budget = 1000.0f;

  strncpy(ministry->minister.name, minister_name, STRING_MEDIUM_LEN - 1);
  ministry->minister.competence = 0.6f;
  ministry->minister.loyalty = 0.7f;

  ministry->proposal_capacity = 4;
  ministry->active_proposals = (civ_reform_proposal_t *)CIV_CALLOC(
      ministry->proposal_capacity, sizeof(civ_reform_proposal_t));

  return ministry;
}

civ_result_t civ_ministry_propose_reform(civ_ministry_t *ministry,
                                         const char *title, const char *desc) {
  civ_result_t result = {CIV_OK, NULL};
  if (!ministry || !title || !desc) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (ministry->proposal_count >= ministry->proposal_capacity) {
    ministry->proposal_capacity *= 2;
    ministry->active_proposals = (civ_reform_proposal_t *)CIV_REALLOC(
        ministry->active_proposals,
        ministry->proposal_capacity * sizeof(civ_reform_proposal_t));
  }

  if (ministry->active_proposals) {
    civ_reform_proposal_t *p =
        &ministry->active_proposals[ministry->proposal_count++];
    strncpy(p->title, title, STRING_MEDIUM_LEN - 1);
    strncpy(p->description, desc, STRING_MAX_LEN - 1);
    p->cost = 500.0f;
    p->impact = 0.1f;
    p->approved = false;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}
