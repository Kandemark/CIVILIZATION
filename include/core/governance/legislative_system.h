/**
 * @file legislative_system.h
 * @brief Dynamic Legislative System
 *
 * Allows for the creation of custom legislative bodies (parliaments, councils,
 * dictators, etc.) that have the power to enact rules and amend constitutions.
 */

#ifndef CIVILIZATION_LEGISLATIVE_SYSTEM_H
#define CIVILIZATION_LEGISLATIVE_SYSTEM_H

#include "../../common.h"
#include "../../types.h"
#include "rule_system.h"

/* Voting Method */
typedef enum {
  CIV_VOTE_METHOD_SIMPLE_MAJORITY = 0, /* > 50% */
  CIV_VOTE_METHOD_SUPER_MAJORITY,      /* > 66% (or custom) */
  CIV_VOTE_METHOD_UNANIMOUS,           /* 100% */
  CIV_VOTE_METHOD_ABSOLUTE_AUTHORITY   /* Leader decides */
} civ_voting_method_t;

/* Legislative Body */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];

  /* Who is in this body? */
  char required_role[STRING_SHORT_LEN]; /* e.g., "Senator", "Councilman" */
  int member_count;                     /* Number of seats */

  /* Powers */
  civ_voting_method_t voting_method;
  civ_float_t custom_threshold; /* For super majority */

  /* Session State */
  bool in_session;
  time_t next_session;
} civ_legislative_body_t;

/* Bill / Proposal */
typedef struct {
  char id[STRING_SHORT_LEN];
  char title[STRING_MEDIUM_LEN];

  civ_rule_t *proposed_rule; /* Rule to add/modify */
  bool is_repeal;            /* If true, remove the rule instead */

  char proposer_id[STRING_SHORT_LEN];

  int votes_yes;
  int votes_no;
  int votes_abstain;

  bool resolved;
  bool passed;
} civ_bill_t;

/* Manager */
typedef struct {
  civ_legislative_body_t *bodies;
  size_t body_count;
  size_t body_capacity;

  civ_bill_t *active_bills;
  size_t bill_count;
  size_t bill_capacity;
} civ_legislative_manager_t;

/* Functions */
civ_legislative_manager_t *civ_legislative_manager_create(void);
void civ_legislative_manager_destroy(civ_legislative_manager_t *manager);

civ_legislative_body_t *civ_legislative_body_create(const char *name,
                                                    const char *required_role);
civ_result_t
civ_legislative_manager_add_body(civ_legislative_manager_t *manager,
                                 civ_legislative_body_t *body);

civ_result_t civ_legislative_propose_bill(civ_legislative_manager_t *manager,
                                          const char *body_id, civ_rule_t *rule,
                                          bool repeal);
civ_result_t civ_legislative_cast_vote(civ_legislative_manager_t *manager,
                                       const char *bill_id,
                                       int vote); /* 1=Yes, -1=No */
civ_result_t
civ_legislative_resolve_bill(civ_legislative_manager_t *manager,
                             const char *bill_id,
                             civ_constitution_t *target_constitution);

#endif /* CIVILIZATION_LEGISLATIVE_SYSTEM_H */
