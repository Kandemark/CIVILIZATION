/**
 * @file relations.c
 * @brief Implementation of diplomacy system
 */

#include "../../../include/core/diplomacy/relations.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_diplomacy_system_t *civ_diplomacy_system_create(void) {
  civ_diplomacy_system_t *ds =
      (civ_diplomacy_system_t *)CIV_MALLOC(sizeof(civ_diplomacy_system_t));
  if (!ds) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate diplomacy system");
    return NULL;
  }

  civ_diplomacy_system_init(ds);
  return ds;
}

void civ_diplomacy_system_destroy(civ_diplomacy_system_t *ds) {
  if (!ds)
    return;

  if (ds->relations) {
    CIV_FREE(ds->relations);
  }

  if (ds->treaties) {
    for (size_t i = 0; i < ds->treaty_count; i++) {
      if (ds->treaties[i].signatories) {
        for (size_t j = 0; j < ds->treaties[i].signatory_count; j++) {
          CIV_FREE(ds->treaties[i].signatories[j]);
        }
        CIV_FREE(ds->treaties[i].signatories);
      }
    }
    CIV_FREE(ds->treaties);
  }

  CIV_FREE(ds);
}

void civ_diplomacy_system_init(civ_diplomacy_system_t *ds) {
  if (!ds)
    return;

  memset(ds, 0, sizeof(civ_diplomacy_system_t));
  ds->relation_capacity = 100;
  ds->relations = (civ_diplomatic_relation_t *)CIV_CALLOC(
      ds->relation_capacity, sizeof(civ_diplomatic_relation_t));
  ds->treaty_capacity = 50;
  ds->treaties =
      (civ_treaty_t *)CIV_CALLOC(ds->treaty_capacity, sizeof(civ_treaty_t));
}

void civ_diplomacy_system_initialize_relations(civ_diplomacy_system_t *ds,
                                               const char **nation_ids,
                                               size_t nation_count) {
  if (!ds || !nation_ids)
    return;

  /* Create relations between all nations */
  for (size_t i = 0; i < nation_count; i++) {
    for (size_t j = 0; j < nation_count; j++) {
      if (i != j) {
        if (ds->relation_count >= ds->relation_capacity) {
          ds->relation_capacity *= 2;
          ds->relations = (civ_diplomatic_relation_t *)CIV_REALLOC(
              ds->relations,
              ds->relation_capacity * sizeof(civ_diplomatic_relation_t));
        }

        civ_diplomatic_relation_t *rel = &ds->relations[ds->relation_count++];
        strncpy(rel->nation_a, nation_ids[i], sizeof(rel->nation_a) - 1);
        strncpy(rel->nation_b, nation_ids[j], sizeof(rel->nation_b) - 1);
        rel->relation_level = CIV_RELATION_LEVEL_NEUTRAL;
        rel->trust = 0.5f;
        rel->personality = (civ_personality_type_t)(rand() % 4);
        rel->last_updated = time(NULL);
      }
    }
  }
}

civ_diplomatic_relation_t *
civ_diplomacy_system_get_relation(civ_diplomacy_system_t *ds,
                                  const char *nation_a, const char *nation_b) {
  if (!ds || !nation_a || !nation_b)
    return NULL;

  for (size_t i = 0; i < ds->relation_count; i++) {
    if ((strcmp(ds->relations[i].nation_a, nation_a) == 0 &&
         strcmp(ds->relations[i].nation_b, nation_b) == 0) ||
        (strcmp(ds->relations[i].nation_a, nation_b) == 0 &&
         strcmp(ds->relations[i].nation_b, nation_a) == 0)) {
      return &ds->relations[i];
    }
  }

  return NULL;
}

void civ_diplomacy_system_update_relations(civ_diplomacy_system_t *ds,
                                           time_t current_date) {
  if (!ds)
    return;

  /* Simplified relation update */
  for (size_t i = 0; i < ds->relation_count; i++) {
    civ_diplomatic_relation_t *rel = &ds->relations[i];

    /* Check for active treaties affecting this relation */
    civ_float_t treaty_bonus = 0.0f;
    for (size_t t = 0; t < ds->treaty_count; t++) {
      civ_treaty_t *treaty = &ds->treaties[t];
      if (treaty->active &&
          treaty->treaty_type == CIV_TREATY_TYPE_TRADE_AGREEMENT) {
        /* Check if this relation involves signatories */
        bool match = false;
        if (strcmp(treaty->signatories[0], rel->nation_a) == 0 &&
            strcmp(treaty->signatories[1], rel->nation_b) == 0)
          match = true;
        if (strcmp(treaty->signatories[0], rel->nation_b) == 0 &&
            strcmp(treaty->signatories[1], rel->nation_a) == 0)
          match = true;

        if (match) {
          treaty_bonus += 0.005f; /* Buff to trust drift */
        }
      }
    }

    /* Gradual drift toward neutral */
    civ_float_t drift = (rel->trust - 0.5f) * 0.01f;
    rel->trust = CLAMP(rel->trust - drift + treaty_bonus, 0.0f, 1.0f);

    /* Update relation level based on trust */
    if (rel->trust < 0.3f) {
      rel->relation_level = CIV_RELATION_LEVEL_HOSTILE;
    } else if (rel->trust < 0.4f) {
      rel->relation_level = CIV_RELATION_LEVEL_NEUTRAL;
    } else if (rel->trust < 0.7f) {
      rel->relation_level = CIV_RELATION_LEVEL_FRIENDLY;
    } else {
      rel->relation_level = CIV_RELATION_LEVEL_ALLIED;
    }

    rel->last_updated = current_date;
  }
}

civ_result_t civ_diplomacy_system_propose_treaty(civ_diplomacy_system_t *ds,
                                                 const char *proposer,
                                                 const char *recipient,
                                                 civ_treaty_type_t type,
                                                 int32_t duration_days) {
  civ_result_t result = {CIV_OK, NULL};

  if (!ds || !proposer || !recipient) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  civ_diplomatic_relation_t *rel =
      civ_diplomacy_system_get_relation(ds, proposer, recipient);
  if (!rel) {
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
  }

  /* Check acceptance based on trust */
  civ_float_t acceptance_chance = rel->trust;
  if (acceptance_chance < 0.3f) {
    result.error = CIV_ERROR_INVALID_STATE;
    result.message = "Treaty rejected due to low trust";
    return result;
  }

  /* Create treaty */
  if (ds->treaty_count >= ds->treaty_capacity) {
    ds->treaty_capacity *= 2;
    ds->treaties = (civ_treaty_t *)CIV_REALLOC(
        ds->treaties, ds->treaty_capacity * sizeof(civ_treaty_t));
  }

  civ_treaty_t *treaty = &ds->treaties[ds->treaty_count++];
  memset(treaty, 0, sizeof(civ_treaty_t));

  snprintf(treaty->treaty_id, sizeof(treaty->treaty_id), "treaty_%zu",
           ds->treaty_count);
  treaty->treaty_type = type;
  treaty->signatory_count = 2;
  treaty->signatories = (char **)CIV_CALLOC(2, sizeof(char *));
  treaty->signatories[0] = (char *)CIV_MALLOC(strlen(proposer) + 1);
  treaty->signatories[1] = (char *)CIV_MALLOC(strlen(recipient) + 1);
  strcpy(treaty->signatories[0], proposer);
  strcpy(treaty->signatories[1], recipient);
  treaty->start_date = time(NULL);
  treaty->duration_days = duration_days;
  treaty->active = true;

  return result;
}

void civ_diplomacy_add_grievance(civ_diplomatic_relation_t *rel,
                                 civ_float_t amount, const char *reason) {
  if (!rel)
    return;

  rel->grievances += amount;
  if (amount > 0.5f && reason) {
    strncpy(rel->primary_casus_belli, reason, STRING_MEDIUM_LEN - 1);
  }
  civ_log(CIV_LOG_INFO, "Grievance added: %s (New total: %.2f)",
          reason ? reason : "Unknown", rel->grievances);
}

bool civ_diplomacy_has_legitimate_war_goal(
    const civ_diplomatic_relation_t *rel) {
  if (!rel)
    return false;

  /* Justified if grievances exceed 1.0 or specific casus belli exists */
  return (rel->grievances > 1.0f) || (strlen(rel->primary_casus_belli) > 0);
}
