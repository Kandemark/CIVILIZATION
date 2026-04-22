/**
 * @file ministry.h
 * @brief Government ministries — NPC-led, budget-driven, with reform powers
 *
 * Each ministry has a minister NPC, a budget, efficiency, and reform system.
 * Ministries compete for budget. No ministry is inherently "good" —
 * every power has tradeoffs: a strong Defence ministry ensures security
 * but drains the treasury; a powerful Finance ministry optimizes tax
 * collection but may strangle growth.
 */
#ifndef CIVILIZATION_MINISTRY_H
#define CIVILIZATION_MINISTRY_H

#include "../../../common.h"
#include "../../../types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CIV_MINISTRY_DEFENCE = 0, CIV_MINISTRY_FINANCE, CIV_MINISTRY_FOREIGN_AFFAIRS,
  CIV_MINISTRY_JUSTICE, CIV_MINISTRY_INTERIOR, CIV_MINISTRY_CULTURE,
  CIV_MINISTRY_SCIENCE, CIV_MINISTRY_TRADE, CIV_MINISTRY_LABOUR,
  CIV_MINISTRY_HEALTH, CIV_MINISTRY_TYPE_COUNT
} civ_ministry_type_t;

typedef struct {
  char id[STRING_SHORT_LEN], name[STRING_MEDIUM_LEN], npc_id[STRING_SHORT_LEN];
  float competence, loyalty, ambition, corruption_vulnerability;
  int   tenure_turns;
} civ_minister_t;

typedef enum { CIV_REFORM_PENDING, CIV_REFORM_APPROVED, CIV_REFORM_REJECTED,
               CIV_REFORM_ACTIVE, CIV_REFORM_COMPLETE } civ_reform_status_t;

typedef struct {
  char id[STRING_SHORT_LEN], title[STRING_MEDIUM_LEN], description[STRING_MAX_LEN];
  float cost, ongoing_cost, impact, implementation_progress;
  civ_reform_status_t status;
  int turns_active;
} civ_reform_proposal_t;

typedef struct {
  civ_ministry_type_t type;
  civ_minister_t      minister;
  float budget, efficiency, corruption_level, power_index, domain_output, public_satisfaction;
  civ_reform_proposal_t *proposals;
  int proposal_count, proposal_capacity, reforms_completed;
} civ_ministry_t;

typedef struct {
  civ_ministry_t *ministries;
  int ministry_count, ministry_capacity;
  float total_budget, budget_pressure;
} civ_ministry_manager_t;

civ_ministry_manager_t *civ_ministry_manager_create(void);
void civ_ministry_manager_destroy(civ_ministry_manager_t *mgr);
civ_ministry_t *civ_ministry_create(civ_ministry_type_t type, const char *minister_name);

void civ_ministry_update_all(civ_ministry_manager_t *mgr, float total_budget,
                             float gov_efficiency, float corruption, float dt);

void civ_ministry_appoint_minister(civ_ministry_t *m, const char *npc_id,
                                   const char *name, float comp, float loy, float amb);
void civ_ministry_remove_minister(civ_ministry_t *m);
bool civ_ministry_minister_may_defect(const civ_ministry_t *m, float legitimacy_crisis);

civ_reform_proposal_t *civ_ministry_propose_reform(civ_ministry_t *m,
                                                    const char *title, const char *desc);
void civ_ministry_process_reforms(civ_ministry_t *m, float available_budget, float dt);

float civ_ministry_defence_output(const civ_ministry_t *m);
float civ_ministry_trade_output(const civ_ministry_t *m);
float civ_ministry_science_output(const civ_ministry_t *m);
civ_ministry_t *civ_ministry_find(const civ_ministry_manager_t *mgr, civ_ministry_type_t type);

#ifdef __cplusplus
}
#endif
#endif
