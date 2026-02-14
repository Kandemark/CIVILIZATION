/**
 * @file international_orgs.c
 * @brief Implementation of international political blocs
 */

#include "../../../include/core/diplomacy/international_orgs.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_international_union_t *civ_org_create(const char *name,
                                          civ_org_category_t category) {
  civ_international_union_t *org = (civ_international_union_t *)CIV_MALLOC(
      sizeof(civ_international_union_t));
  if (!org)
    return NULL;

  memset(org, 0, sizeof(civ_international_union_t));
  strncpy(org->name, name, STRING_MEDIUM_LEN - 1);
  org->category = category;
  org->member_capacity = 8;
  org->member_nation_ids =
      (char **)CIV_CALLOC(org->member_capacity, sizeof(char *));
  org->institutional_cohesion = 0.5f;

  return org;
}

void civ_org_destroy(civ_international_union_t *org) {
  if (!org)
    return;
  for (size_t i = 0; i < org->member_count; i++) {
    CIV_FREE(org->member_nation_ids[i]);
  }
  CIV_FREE(org->member_nation_ids);
  CIV_FREE(org);
}

civ_result_t civ_org_add_member(civ_international_union_t *org,
                                const char *nation_id) {
  if (!org || !nation_id)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  if (org->member_count >= org->member_capacity) {
    org->member_capacity *= 2;
    org->member_nation_ids = (char **)CIV_REALLOC(
        org->member_nation_ids, org->member_capacity * sizeof(char *));
  }

  if (org->member_nation_ids) {
    org->member_nation_ids[org->member_count] =
        (char *)CIV_MALLOC(STRING_SHORT_LEN);
    strncpy(org->member_nation_ids[org->member_count++], nation_id,
            STRING_SHORT_LEN - 1);
    return (civ_result_t){CIV_OK, NULL};
  }

  return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
}

void civ_org_apply_sanction(civ_international_union_t *org,
                            const char *target_nation_id) {
  if (!org || !target_nation_id)
    return;

  /* Log sanction event - would affect trade volumes in a full loop */
  civ_log(CIV_LOG_WARNING,
          "Organization %s has applied economic sanctions to %s", org->name,
          target_nation_id);
  org->institutional_cohesion -= 0.05f * (1.0f - org->institutional_cohesion);
}
