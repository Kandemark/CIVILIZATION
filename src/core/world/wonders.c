/**
 * @file wonders.c
 * @brief Implementation of the Wonders system
 */

#include "../../../include/core/world/wonders.h"
#include <stdlib.h>
#include <string.h>


civ_wonder_manager_t *civ_wonder_manager_create(void) {
  civ_wonder_manager_t *manager =
      (civ_wonder_manager_t *)CIV_MALLOC(sizeof(civ_wonder_manager_t));
  if (manager) {
    civ_wonder_manager_init(manager);
  }
  return manager;
}

void civ_wonder_manager_destroy(civ_wonder_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager);
  }
}

void civ_wonder_manager_init(civ_wonder_manager_t *manager) {
  if (!manager)
    return;

  memset(manager, 0, sizeof(civ_wonder_manager_t));

  /* Pyramids: Boosts Production */
  {
    civ_wonder_t *w = &manager->wonders[CIV_WONDER_PYRAMIDS];
    strncpy(w->name, "The Pyramids", STRING_MEDIUM_LEN - 1);
    strncpy(w->description,
            "Giant stone monuments that boost worker efficiency.",
            STRING_MAX_LEN - 1);
    w->production_cost = 200.0f;
    strncpy(w->required_tech_id, "masonry", STRING_SHORT_LEN - 1);
    w->effects.production_mult = 0.15f; /* +15% Global Production */
  }

  /* Great Library: Boosts Science */
  {
    civ_wonder_t *w = &manager->wonders[CIV_WONDER_GREAT_LIBRARY];
    strncpy(w->name, "The Great Library", STRING_MEDIUM_LEN - 1);
    strncpy(w->description, "A repository of all world knowledge.",
            STRING_MAX_LEN - 1);
    w->production_cost = 250.0f;
    strncpy(w->required_tech_id, "writing", STRING_SHORT_LEN - 1);
    w->effects.science_mult = 0.20f; /* +20% Science */
  }

  /* Colossus: Boosts Gold */
  {
    civ_wonder_t *w = &manager->wonders[CIV_WONDER_COLOSSUS];
    strncpy(w->name, "The Colossus", STRING_MEDIUM_LEN - 1);
    strncpy(w->description, "A massive copper statue guarding the harbor.",
            STRING_MAX_LEN - 1);
    w->production_cost = 180.0f;
    strncpy(w->required_tech_id, "bronze_working", STRING_SHORT_LEN - 1);
    w->requires_coast = true;
    w->effects.gold_mult = 0.25f; /* +25% Gold */
  }

  /* Hanging Gardens: Boosts Population/Growth (using Production mult for now)
   */
  {
    civ_wonder_t *w = &manager->wonders[CIV_WONDER_HANGING_GARDENS];
    strncpy(w->name, "Hanging Gardens", STRING_MEDIUM_LEN - 1);
    strncpy(w->description, "Lush terraced gardens in the heart of the city.",
            STRING_MAX_LEN - 1);
    w->production_cost = 220.0f;
    strncpy(w->required_tech_id, "irrigation", STRING_SHORT_LEN - 1);
    w->effects.production_mult = 0.10f;
  }
}

bool civ_wonder_can_build(const civ_wonder_manager_t *manager,
                          civ_wonder_type_t type, const char *tech_id,
                          bool is_coastal) {
  if (!manager || type >= CIV_WONDER_COUNT)
    return false;

  const civ_wonder_t *w = &manager->wonders[type];
  if (w->is_built)
    return false;

  /* Tech requirement */
  if (tech_id && strcmp(w->required_tech_id, tech_id) != 0)
    return false;

  /* Coast requirement */
  if (w->requires_coast && !is_coastal)
    return false;

  return true;
}

void civ_wonder_mark_built(civ_wonder_manager_t *manager,
                           civ_wonder_type_t type, const char *builder_id) {
  if (!manager || type >= CIV_WONDER_COUNT)
    return;

  civ_wonder_t *w = &manager->wonders[type];
  w->is_built = true;
  if (builder_id) {
    strncpy(w->builder_id, builder_id, STRING_SHORT_LEN - 1);
  }
}

civ_wonder_effects_t
civ_wonder_calculate_global_bonuses(const civ_wonder_manager_t *manager,
                                    const char *owner_id) {
  civ_wonder_effects_t total = {0};
  if (!manager || !owner_id)
    return total;

  for (int i = 0; i < CIV_WONDER_COUNT; i++) {
    const civ_wonder_t *w = &manager->wonders[i];
    if (w->is_built && strcmp(w->builder_id, owner_id) == 0) {
      total.science_mult += w->effects.science_mult;
      total.culture_mult += w->effects.culture_mult;
      total.production_mult += w->effects.production_mult;
      total.gold_mult += w->effects.gold_mult;
      total.military_str_bonus += w->effects.military_str_bonus;
    }
  }

  return total;
}
