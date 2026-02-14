/**
 * @file institution.c
 * @brief Implementation of modular institutional framework
 */

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "../../../include/core/governance/institution.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_institution_manager_t *civ_institution_manager_create(void) {
  civ_institution_manager_t *manager = (civ_institution_manager_t *)CIV_MALLOC(
      sizeof(civ_institution_manager_t));
  if (manager) {
    manager->count = 0;
    manager->capacity = 8;
    manager->items = (civ_institution_t *)CIV_MALLOC(sizeof(civ_institution_t) *
                                                     manager->capacity);
  }
  return manager;
}

void civ_institution_manager_destroy(civ_institution_manager_t *manager) {
  if (manager) {
    if (manager->items)
      CIV_FREE(manager->items);
    CIV_FREE(manager);
  }
}

civ_result_t civ_institution_found(civ_institution_manager_t *manager,
                                   const char *name, uint32_t focuses,
                                   const char *governing_role) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null manager"};

  if (manager->count >= manager->capacity) {
    size_t new_cap = manager->capacity * 2;
    civ_institution_t *new_items = (civ_institution_t *)realloc(
        manager->items, sizeof(civ_institution_t) * new_cap);
    if (!new_items)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->items = new_items;
    manager->capacity = new_cap;
  }

  civ_institution_t *inst = &manager->items[manager->count++];
  memset(inst, 0, sizeof(civ_institution_t));

  strncpy(inst->name, name, STRING_MEDIUM_LEN - 1);
  inst->focuses = focuses;
  if (governing_role) {
    strncpy(inst->governing_role, governing_role, STRING_SHORT_LEN - 1);
  }

  inst->stature = 1.0f; /* Starting point */
  inst->is_active = true;

  snprintf(inst->id, STRING_SHORT_LEN, "inst_%zu", manager->count);

  return (civ_result_t){CIV_OK, NULL};
}

void civ_institution_update(civ_institution_manager_t *manager,
                            civ_float_t budget_total,
                            civ_float_t gov_efficiency,
                            civ_float_t time_delta) {
  if (!manager || manager->count == 0)
    return;

  /* Allocation per active institution (simplistic equal split for now) */
  civ_float_t active_count = 0;
  for (size_t i = 0; i < manager->count; i++) {
    if (manager->items[i].is_active && !manager->items[i].is_dissolving) {
      active_count += 1.0f;
    }
  }

  if (active_count == 0)
    return;

  civ_float_t budget_per_inst = budget_total / active_count;

  for (size_t i = 0; i < manager->count; i++) {
    civ_institution_t *inst = &manager->items[i];
    if (!inst->is_active)
      continue;

    if (inst->is_dissolving) {
      inst->stature -= 0.1f * time_delta;
      if (inst->stature <= 0.0f) {
        inst->stature = 0.0f;
        inst->is_active = false;
      }
      continue;
    }

    /* Stature growth: Funded level * efficiency */
    /* Maintenance cost scales exponentially: higher stature is harder to keep
     */
    inst->maintenance_cost = 0.05f * powf(inst->stature, 1.2f);

    civ_float_t net_investment = budget_per_inst - inst->maintenance_cost;

    /* Growth or Decay based on net investment */
    civ_float_t drift = net_investment * gov_efficiency * 0.01f;
    inst->stature = fmaxf(0.1f, inst->stature + drift * time_delta);
  }
}

civ_float_t
civ_institution_get_total_bonus(const civ_institution_manager_t *manager,
                                civ_institution_focus_t focus) {
  if (!manager)
    return 0.0f;

  civ_float_t total_bonus = 0.0f;
  for (size_t i = 0; i < manager->count; i++) {
    const civ_institution_t *inst = &manager->items[i];
    if (inst->is_active && (inst->focuses & focus)) {
      /* Logarithmic benefit scaling: Benefit = log2(1 + stature) * 0.1 */
      total_bonus += log2f(1.0f + inst->stature) * 0.1f;
    }
  }

  return total_bonus;
}
