/**
 * @file political_rivalry.h
 * @brief Political Rivalry, Coups, and Opposition Management
 */

#ifndef CIVILIZATION_POLITICAL_RIVALRY_H
#define CIVILIZATION_POLITICAL_RIVALRY_H

#include "../../common.h"
#include "../../types.h"

/* Opposition Action Type */
typedef enum {
  CIV_OPP_CHARITY = 0,  /* Gain local popularity */
  CIV_OPP_PROPAGANDA,   /* Damage leader trust */
  CIV_OPP_COUP_ATTEMPT, /* Attempt to seize power */
  CIV_OPP_PROTEST,      /* Increase unrest */
  CIV_OPP_CONCESSION    /* Negotiate for minor power */
} civ_opposition_action_t;

/* Leadership Suppression Level */
typedef enum {
  CIV_SUP_TOLERANCE = 0,
  CIV_SUP_SURVEILLANCE,
  CIV_SUP_ARRESTS,
  CIV_SUP_ELIMINATION
} civ_suppression_level_t;

/* Rival Stats */
typedef struct {
  char name[STRING_SHORT_LEN];
  civ_float_t popularity; /* 0.0 to 1.0 */
  civ_float_t influence;  /* Resource power */
  civ_float_t radicalism; /* Tendency to use violence */
  bool is_active;
} civ_political_rival_t;

/* Functions */
void civ_rivalry_init_rival(civ_political_rival_t *rival, const char *name);
civ_result_t
civ_rivalry_perform_opposition_action(civ_political_rival_t *player,
                                      civ_opposition_action_t action);
civ_result_t civ_rivalry_apply_suppression(civ_suppression_level_t level,
                                           civ_political_rival_t *target);

#endif /* CIVILIZATION_POLITICAL_RIVALRY_H */
