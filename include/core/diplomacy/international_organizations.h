/**
 * @file international_organizations.h
 * @brief International Organizations and Alliances System
 */

#ifndef CIVILIZATION_INTERNATIONAL_ORGANIZATIONS_H
#define CIVILIZATION_INTERNATIONAL_ORGANIZATIONS_H

#include "../../common.h"
#include "../../types.h"
#include "../governance/rule_system.h"

/* Organization Type */
typedef enum {
  CIV_ORG_TYPE_ALLIANCE = 0,    /* Military/Strategic Alliance */
  CIV_ORG_TYPE_TRADE_BLOC,      /* Economic Union */
  CIV_ORG_TYPE_FEDERATION,      /* Political Federation */
  CIV_ORG_TYPE_SCIENTIFIC_COOP, /* Research Group */
  CIV_ORG_TYPE_GLOBAL_FORUM     /* UN-style body */
} civ_org_type_t;

/* Resolution */
typedef struct {
  char id[STRING_SHORT_LEN];
  char title[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];
  char proposer_id[STRING_SHORT_LEN];

  int votes_for;
  int votes_against;
  int votes_abstain;

  bool passed;
  bool active;
} civ_resolution_t;

/* International Organization */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_org_type_t type;

  char **member_ids;
  size_t member_count;
  size_t member_capacity;

  char leader_id[STRING_SHORT_LEN]; /* Optional leader/chair */

  civ_resolution_t *resolutions;
  size_t resolution_count;
  size_t resolution_capacity;

  civ_float_t cohesion; /* 0.0 to 1.0 */
  time_t formation_date;

  civ_constitution_t *charter; /* The organization's charter/constitution */
} civ_international_org_t;

/* Manager */
typedef struct {
  civ_international_org_t *orgs;
  size_t org_count;
  size_t org_capacity;
} civ_org_manager_t;

/* Functions */
civ_org_manager_t *civ_org_manager_create(void);
void civ_org_manager_destroy(civ_org_manager_t *manager);

civ_international_org_t *civ_org_create(const char *name, civ_org_type_t type);
void civ_org_destroy(civ_international_org_t *org);

civ_result_t civ_org_add_member(civ_international_org_t *org,
                                const char *nation_id);
civ_result_t civ_org_remove_member(civ_international_org_t *org,
                                   const char *nation_id);
civ_result_t civ_org_propose_resolution(civ_international_org_t *org,
                                        const char *title, const char *desc,
                                        const char *proposer);
civ_result_t civ_org_vote(civ_international_org_t *org,
                          const char *resolution_id, const char *voter_id,
                          int vote); /* 1=For, -1=Against, 0=Abstain */

civ_result_t civ_org_manager_add(civ_org_manager_t *manager,
                                 civ_international_org_t *org);
civ_international_org_t *civ_org_manager_find(civ_org_manager_t *manager,
                                              const char *id);

#endif /* CIVILIZATION_INTERNATIONAL_ORGANIZATIONS_H */
