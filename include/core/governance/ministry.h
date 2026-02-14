/**
 * @file ministry.h
 * @brief Ministry and Minister system
 */

#ifndef CIVILIZATION_MINISTRY_H
#define CIVILIZATION_MINISTRY_H

#include "../../common.h"
#include "../../types.h"

/* Ministry Types */
typedef enum {
  CIV_MINISTRY_DEFENCE = 0,
  CIV_MINISTRY_FINANCE,
  CIV_MINISTRY_FOREIGN_AFFAIRS,
  CIV_MINISTRY_JUSTICE,
  CIV_MINISTRY_INTERIOR,
  CIV_MINISTRY_CULTURE,
  CIV_MINISTRY_SCIENCE
} civ_ministry_type_t;

/* Minister */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_float_t loyalty;
  civ_float_t competence;
  civ_float_t ambition;
  char native_language_id[STRING_SHORT_LEN];
} civ_minister_t;

/* Reform Proposal */
typedef struct {
  char id[STRING_SHORT_LEN];
  char title[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];
  civ_float_t cost;
  civ_float_t impact;
  bool approved;
} civ_reform_proposal_t;

/* Ministry */
typedef struct {
  civ_ministry_type_t type;
  civ_minister_t minister;
  civ_float_t budget;
  civ_float_t efficiency;

  civ_reform_proposal_t *active_proposals;
  size_t proposal_count;
  size_t proposal_capacity;
} civ_ministry_t;

/* Ministry Manager */
typedef struct {
  civ_ministry_t *ministries;
  size_t ministry_count;
  size_t ministry_capacity;
} civ_ministry_manager_t;

/* Functions */
civ_ministry_manager_t *civ_ministry_manager_create(void);
void civ_ministry_manager_destroy(civ_ministry_manager_t *manager);

civ_ministry_t *civ_ministry_create(civ_ministry_type_t type,
                                    const char *minister_name);
civ_result_t civ_ministry_propose_reform(civ_ministry_t *ministry,
                                         const char *title, const char *desc);

#endif /* CIVILIZATION_MINISTRY_H */
