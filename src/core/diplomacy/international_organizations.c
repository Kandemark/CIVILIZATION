/**
 * @file international_organizations.c
 * @brief Implementation of International Organizations System
 */

#include "../../../include/core/diplomacy/international_organizations.h"
#include <stdio.h>

civ_org_manager_t *civ_org_manager_create(void) {
  civ_org_manager_t *manager = CIV_MALLOC(sizeof(civ_org_manager_t));
  if (manager) {
    manager->orgs = NULL;
    manager->org_count = 0;
    manager->org_capacity = 0;
  }
  return manager;
}

void civ_org_manager_destroy(civ_org_manager_t *manager) {
  if (manager) {
    if (manager->orgs) {
      for (size_t i = 0; i < manager->org_count; i++) {
        civ_org_destroy(&manager->orgs[i]);
      }
      CIV_FREE(manager->orgs);
    }
    CIV_FREE(manager);
  }
}

civ_international_org_t *civ_org_create(const char *name, civ_org_type_t type) {
  civ_international_org_t *org = CIV_MALLOC(sizeof(civ_international_org_t));
  if (org) {
    snprintf(org->id, STRING_SHORT_LEN, "org_%ld", (long)time(NULL));
    strncpy(org->name, name, STRING_MEDIUM_LEN - 1);
    org->type = type;

    org->member_ids = NULL;
    org->member_count = 0;
    org->member_capacity = 0;

    org->resolutions = NULL;
    org->resolution_count = 0;
    org->resolution_capacity = 0;

    org->cohesion = 1.0f;
    org->formation_date = time(NULL);
    memset(org->leader_id, 0, STRING_SHORT_LEN);
  }
  return org;
}

void civ_org_destroy(civ_international_org_t *org) {
  if (org) {
    if (org->member_ids) {
      for (size_t i = 0; i < org->member_count; i++) {
        CIV_FREE(org->member_ids[i]);
      }
      CIV_FREE(org->member_ids);
    }
    CIV_FREE(org->resolutions);
    // Note: if org was malloced separately vs in array, handle accordingly.
    // Assuming here it's used with manager which copies, or standalone.
    // If standalone, we free org. If in manager, manager frees array.
    // This function cleans CONTENT.
  }
}

civ_result_t civ_org_add_member(civ_international_org_t *org,
                                const char *nation_id) {
  if (!org || !nation_id)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  // Check duplicate
  for (size_t i = 0; i < org->member_count; i++) {
    if (strcmp(org->member_ids[i], nation_id) == 0) {
      return (civ_result_t){CIV_ERROR_INVALID_STATE, "Already a member"};
    }
  }

  if (org->member_count >= org->member_capacity) {
    size_t new_cap = org->member_capacity == 0 ? 4 : org->member_capacity * 2;
    char **new_ids = CIV_REALLOC(org->member_ids, new_cap * sizeof(char *));
    if (!new_ids)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    org->member_ids = new_ids;
    org->member_capacity = new_cap;
  }

  org->member_ids[org->member_count] = CIV_MALLOC(strlen(nation_id) + 1);
  strcpy(org->member_ids[org->member_count], nation_id);
  org->member_count++;

  return (civ_result_t){CIV_OK, "Member added"};
}

civ_result_t civ_org_propose_resolution(civ_international_org_t *org,
                                        const char *title, const char *desc,
                                        const char *proposer) {
  if (!org || !title)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (org->resolution_count >= org->resolution_capacity) {
    size_t new_cap =
        org->resolution_capacity == 0 ? 4 : org->resolution_capacity * 2;
    civ_resolution_t *new_res =
        CIV_REALLOC(org->resolutions, new_cap * sizeof(civ_resolution_t));
    if (!new_res)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    org->resolutions = new_res;
    org->resolution_capacity = new_cap;
  }

  civ_resolution_t *res = &org->resolutions[org->resolution_count++];
  snprintf(res->id, STRING_SHORT_LEN, "res_%ld", (long)time(NULL));
  strncpy(res->title, title, STRING_MEDIUM_LEN - 1);
  if (desc)
    strncpy(res->description, desc, STRING_MAX_LEN - 1);
  if (proposer)
    strncpy(res->proposer_id, proposer, STRING_SHORT_LEN - 1);

  res->votes_for = 0;
  res->votes_against = 0;
  res->votes_abstain = 0;
  res->passed = false;
  res->active = true;

  return (civ_result_t){CIV_OK, "Resolution proposed"};
}

civ_result_t civ_org_manager_add(civ_org_manager_t *manager,
                                 civ_international_org_t *org) {
  if (!manager || !org)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->org_count >= manager->org_capacity) {
    size_t new_cap = manager->org_capacity == 0 ? 4 : manager->org_capacity * 2;
    civ_international_org_t *new_orgs =
        CIV_REALLOC(manager->orgs, new_cap * sizeof(civ_international_org_t));
    if (!new_orgs)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->orgs = new_orgs;
    manager->org_capacity = new_cap;
  }

  // Shallow copy struct, but ownership of deep pointers (members, resolutions)
  // transfers to manager array BE CAREFUL: if 'org' was malloced, we must free
  // the container but NOT the contents
  manager->orgs[manager->org_count++] = *org;

  return (civ_result_t){CIV_OK, "Organization added"};
}
