/**
 * @file subdivision.c
 * @brief Implementation of administrative subdivision hierarchy
 */

#include "../../../include/core/governance/subdivision.h"
#include <stdlib.h>
#include <string.h>

civ_subdivision_manager_t *civ_subdivision_manager_create(void) {
  civ_subdivision_manager_t *manager = (civ_subdivision_manager_t *)CIV_MALLOC(
      sizeof(civ_subdivision_manager_t));
  if (manager) {
    manager->count = 0;
    manager->capacity = 4;
    manager->items = (civ_subdivision_t *)CIV_MALLOC(sizeof(civ_subdivision_t) *
                                                     manager->capacity);
  }
  return manager;
}

void civ_subdivision_manager_destroy(civ_subdivision_manager_t *manager) {
  if (manager) {
    for (size_t i = 0; i < manager->count; i++) {
      if (manager->items[i].tile_indices)
        CIV_FREE(manager->items[i].tile_indices);
      if (manager->items[i].settlement_ids) {
        for (size_t j = 0; j < manager->items[i].settlement_count; j++) {
          CIV_FREE(manager->items[i].settlement_ids[j]);
        }
        CIV_FREE(manager->items[i].settlement_ids);
      }
    }
    CIV_FREE(manager->items);
    CIV_FREE(manager);
  }
}

civ_subdivision_t *civ_subdivision_create(civ_subdivision_manager_t *manager,
                                          const char *name,
                                          civ_subdivision_type_t type) {
  if (!manager)
    return NULL;

  if (manager->count >= manager->capacity) {
    size_t new_cap = manager->capacity * 2;
    civ_subdivision_t *new_items = (civ_subdivision_t *)realloc(
        manager->items, sizeof(civ_subdivision_t) * new_cap);
    if (!new_items)
      return NULL;
    manager->items = new_items;
    manager->capacity = new_cap;
  }

  civ_subdivision_t *sub = &manager->items[manager->count++];
  memset(sub, 0, sizeof(civ_subdivision_t));

  strncpy(sub->name, name, STRING_MEDIUM_LEN - 1);
  sub->type = type;
  sub->autonomy = (type == CIV_SUBDIVISION_COLONY) ? 0.6f : 0.1f;
  sub->stability = 1.0f;

  sub->tile_capacity = 64;
  sub->tile_indices =
      (uint32_t *)CIV_MALLOC(sizeof(uint32_t) * sub->tile_capacity);

  snprintf(sub->id, STRING_SHORT_LEN, "sub_%zu", manager->count);

  return sub;
}

void civ_subdivision_add_tile(civ_subdivision_t *sub, uint32_t tile_index) {
  if (!sub)
    return;

  if (sub->tile_count >= sub->tile_capacity) {
    size_t new_cap = sub->tile_capacity * 2;
    uint32_t *new_tiles =
        (uint32_t *)realloc(sub->tile_indices, sizeof(uint32_t) * new_cap);
    if (!new_tiles)
      return;
    sub->tile_indices = new_tiles;
    sub->tile_capacity = new_cap;
  }

  sub->tile_indices[sub->tile_count++] = tile_index;
}

void civ_subdivision_add_settlement(civ_subdivision_t *sub,
                                    const char *settlement_id) {
  if (!sub || !settlement_id)
    return;

  /* Lazy init of settlement IDs list */
  if (!sub->settlement_ids) {
    sub->settlement_ids = (char **)CIV_MALLOC(sizeof(char *) * 8);
  }

  sub->settlement_ids[sub->settlement_count++] = strdup(settlement_id);
}

void civ_subdivision_update(civ_subdivision_manager_t *manager,
                            civ_float_t time_delta) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->count; i++) {
    civ_subdivision_t *sub = &manager->items[i];

    /* Stability drift based on type and autonomy mismatch */
    civ_float_t target_stability = 1.0f - (sub->autonomy * 0.2f);
    if (sub->type == CIV_SUBDIVISION_OCCUPIED)
      target_stability = 0.4f;

    sub->stability += (target_stability - sub->stability) * 0.05f * time_delta;
  }
}
