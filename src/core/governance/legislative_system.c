/**
 * @file legislative_system.c
 * @brief Implementation of Dynamic Legislative System
 */

#include "../../../include/core/governance/legislative_system.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

civ_legislative_manager_t *civ_legislative_manager_create(void) {
  civ_legislative_manager_t *manager =
      CIV_MALLOC(sizeof(civ_legislative_manager_t));
  if (manager) {
    manager->bodies = NULL;
    manager->body_count = 0;
    manager->body_capacity = 0;

    manager->active_bills = NULL;
    manager->bill_count = 0;
    manager->bill_capacity = 0;
  }
  return manager;
}

void civ_legislative_manager_destroy(civ_legislative_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->bodies);
    // Deep clean bills if needed (free proposed_rule)
    for (size_t i = 0; i < manager->bill_count; i++) {
      civ_rule_destroy(manager->active_bills[i].proposed_rule);
    }
    CIV_FREE(manager->active_bills);
    CIV_FREE(manager);
  }
}

civ_legislative_body_t *civ_legislative_body_create(const char *name,
                                                    const char *required_role) {
  civ_legislative_body_t *body = CIV_MALLOC(sizeof(civ_legislative_body_t));
  if (body) {
    snprintf(body->id, STRING_SHORT_LEN, "leg_%ld", (long)time(NULL));
    strncpy(body->name, name, STRING_MEDIUM_LEN - 1);

    if (required_role)
      strncpy(body->required_role, required_role, STRING_SHORT_LEN - 1);
    else
      memset(body->required_role, 0, STRING_SHORT_LEN);

    body->member_count = 0;
    body->session_type = CIV_SESSION_LEGISLATIVE;
    body->voting_method = CIV_VOTE_METHOD_SIMPLE_MAJORITY;
    body->custom_threshold = 0.5f;
    body->in_session = true;
    body->next_session = 0;
  }
  return body;
}

civ_result_t
civ_legislative_manager_add_body(civ_legislative_manager_t *manager,
                                 civ_legislative_body_t *body) {
  if (!manager || !body)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->body_count >= manager->body_capacity) {
    size_t new_cap =
        manager->body_capacity == 0 ? 4 : manager->body_capacity * 2;
    civ_legislative_body_t *new_bodies =
        CIV_REALLOC(manager->bodies, new_cap * sizeof(civ_legislative_body_t));
    if (!new_bodies)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->bodies = new_bodies;
    manager->body_capacity = new_cap;
  }

  manager->bodies[manager->body_count++] = *body;
  // Note: body pointer passed in is now copied, caller should free container if
  // malloced
  return (civ_result_t){CIV_OK, "Body added"};
}

civ_result_t civ_legislative_propose_bill(civ_legislative_manager_t *manager,
                                          const char *body_id, civ_rule_t *rule,
                                          bool repeal) {
  if (!manager || !body_id || !rule)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->bill_count >= manager->bill_capacity) {
    size_t new_cap =
        manager->bill_capacity == 0 ? 8 : manager->bill_capacity * 2;
    civ_bill_t *new_bills =
        CIV_REALLOC(manager->active_bills, new_cap * sizeof(civ_bill_t));
    if (!new_bills)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->active_bills = new_bills;
    manager->bill_capacity = new_cap;
  }

  civ_bill_t *bill = &manager->active_bills[manager->bill_count++];
  snprintf(bill->id, STRING_SHORT_LEN, "bill_%ld", (long)time(NULL));
  snprintf(bill->title, STRING_MEDIUM_LEN, "%s %s", repeal ? "Repeal" : "Enact",
           rule->name);

  bill->proposed_rule = rule; // Takes ownership
  bill->is_repeal = repeal;
  bill->votes_yes = 0;
  bill->votes_no = 0;
  bill->votes_abstain = 0;
  bill->resolved = false;
  bill->passed = false;

  return (civ_result_t){CIV_OK, "Bill proposed"};
}

civ_result_t civ_legislative_cast_vote(civ_legislative_manager_t *manager,
                                       const char *bill_id, int vote) {
  if (!manager || !bill_id)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  for (size_t i = 0; i < manager->bill_count; i++) {
    if (strcmp(manager->active_bills[i].id, bill_id) == 0) {
      if (vote > 0)
        manager->active_bills[i].votes_yes++;
      else if (vote < 0)
        manager->active_bills[i].votes_no++;
      else
        manager->active_bills[i].votes_abstain++;
      return (civ_result_t){CIV_OK, "Vote cast"};
    }
  }
  return (civ_result_t){CIV_ERROR_NOT_FOUND, "Bill not found"};
}

civ_result_t
civ_legislative_resolve_bill(civ_legislative_manager_t *manager,
                             const char *bill_id,
                             civ_constitution_t *target_constitution) {
  if (!manager || !bill_id || !target_constitution)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  for (size_t i = 0; i < manager->bill_count; i++) {
    civ_bill_t *bill = &manager->active_bills[i];
    if (strcmp(bill->id, bill_id) == 0) {
      if (bill->resolved)
        return (civ_result_t){CIV_ERROR_INVALID_STATE, "Already resolved"};

      // Simple majority check for now. Real logic would use body's voting
      // method.
      int total_votes = bill->votes_yes + bill->votes_no;
      if (total_votes > 0 && (float)bill->votes_yes / total_votes > 0.5f) {
        bill->passed = true;
        if (bill->is_repeal) {
          civ_constitution_remove_rule(target_constitution,
                                       bill->proposed_rule->id);
        } else {
          civ_constitution_add_rule(target_constitution, bill->proposed_rule);
        }
      } else {
        bill->passed = false;
      }

      bill->resolved = true;
      return (civ_result_t){CIV_OK,
                            bill->passed ? "Bill passed" : "Bill failed"};
    }
  }
  return (civ_result_t){CIV_ERROR_NOT_FOUND, "Bill not found"};
}
