/**
 * @file international_orgs.h
 * @brief International Organizations and Political Blocs
 */

#ifndef CIVILIZATION_INTERNATIONAL_ORGS_H
#define CIVILIZATION_INTERNATIONAL_ORGS_H

#include "../../common.h"
#include "../../types.h"
#include "relations.h"

/* Organization Category */
typedef enum {
  CIV_ORG_TRADE_BLOC = 0,    /* WTO / Hanseatic League type */
  CIV_ORG_DEFENSE_UNION,     /* NATO type */
  CIV_ORG_RELIGIOUS_COUNCIL, /* Holy League type */
  CIV_ORG_WORLD_ASSEMBLY     /* UN type */
} civ_org_category_t;

/* International Body */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_org_category_t category;

  char **member_nation_ids;
  size_t member_count;
  size_t member_capacity;

  civ_float_t institutional_cohesion; /* 0.0 to 1.0 */
} civ_international_union_t;

/* Functions */
civ_international_union_t *civ_org_create(const char *name,
                                          civ_org_category_t category);
void civ_org_destroy(civ_international_union_t *org);
civ_result_t civ_org_add_member(civ_international_union_t *org,
                                const char *nation_id);

void civ_org_apply_sanction(civ_international_union_t *org,
                            const char *target_nation_id);

#endif /* CIVILIZATION_INTERNATIONAL_ORGS_H */
