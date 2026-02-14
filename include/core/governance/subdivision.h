/**
 * @file subdivision.h
 * @brief Administrative Hierarchy (States, Colonies, Provinces)
 */

#ifndef CIVILIZATION_SUBDIVISION_H
#define CIVILIZATION_SUBDIVISION_H

#include "../../common.h"
#include "../../types.h"

/* Subdivision Type */
typedef enum {
  CIV_SUBDIVISION_STATE = 0, /* Fully incorporated */
  CIV_SUBDIVISION_COLONY,    /* Distant, high autonomy/friction */
  CIV_SUBDIVISION_PROVINCE,  /* Military/Administrative district */
  CIV_SUBDIVISION_OCCUPIED   /* Captured territory under transition */
} civ_subdivision_type_t;

/* Subdivision Structure */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_subdivision_type_t type;

  civ_float_t autonomy;  /* 0.0 (Direct Rule) to 1.0 (High Autonomy) */
  civ_float_t stability; /* Local stability metric */

  /* Territorial Definition */
  uint32_t *tile_indices;
  size_t tile_count;
  size_t tile_capacity;

  /* Settlement Membership (subset of the nation's settlements) */
  char **settlement_ids;
  size_t settlement_count;
} civ_subdivision_t;

/* Manager */
typedef struct {
  civ_subdivision_t *items;
  size_t count;
  size_t capacity;
} civ_subdivision_manager_t;

/* Functions */
civ_subdivision_manager_t *civ_subdivision_manager_create(void);
void civ_subdivision_manager_destroy(civ_subdivision_manager_t *manager);

civ_subdivision_t *civ_subdivision_create(civ_subdivision_manager_t *manager,
                                          const char *name,
                                          civ_subdivision_type_t type);

void civ_subdivision_add_tile(civ_subdivision_t *sub, uint32_t tile_index);
void civ_subdivision_add_settlement(civ_subdivision_t *sub,
                                    const char *settlement_id);

void civ_subdivision_update(civ_subdivision_manager_t *manager,
                            civ_float_t time_delta);

#endif /* CIVILIZATION_SUBDIVISION_H */
