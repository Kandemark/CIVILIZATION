/**
 * @file corruption.h
 * @brief Systemic Corruption Engine
 */

#ifndef CIVILIZATION_CORRUPTION_H
#define CIVILIZATION_CORRUPTION_H

#include "../../common.h"
#include "../../types.h"

/* Corruption Type */
typedef enum {
  CIV_CORRUPTION_GRAFT = 0,    /* Bribery/Kickbacks */
  CIV_CORRUPTION_NEPOTISM,     /* Family/Friend favoritism */
  CIV_CORRUPTION_EMBEZZLEMENT, /* Stealing from budget */
  CIV_CORRUPTION_EXTORTION,    /* Using power to coerce */
  CIV_CORRUPTION_PATRONAGE     /* Trading jobs for loyalty */
} civ_corruption_type_t;

/* Corruption Network Node */
typedef struct {
  char npc_id[STRING_SHORT_LEN];
  civ_float_t involvement; /* 0.0 to 1.0 */
  civ_float_t influence;
  char cliques[4][STRING_SHORT_LEN]; /* Names of secret groups */
  size_t clique_count;
} civ_corruption_node_t;

/* Corruption System */
typedef struct {
  civ_corruption_node_t *nodes;
  size_t node_count;
  size_t node_capacity;

  civ_float_t systemic_index; /* Overall national corruption */
  civ_float_t shadow_budget;  /* Funds diverted from national treasury */

  civ_float_t audit_effectiveness; /* How hard it is to hide corruption */
} civ_corruption_engine_t;

/* Functions */
civ_corruption_engine_t *civ_corruption_engine_create(void);
void civ_corruption_engine_destroy(civ_corruption_engine_t *engine);

civ_result_t civ_corruption_add_involvement(civ_corruption_engine_t *engine,
                                            const char *npc_id,
                                            civ_float_t amount);
civ_float_t
civ_corruption_calculate_leakage(const civ_corruption_engine_t *engine,
                                 civ_float_t total_budget);

/* Detection & Suppression */
civ_result_t civ_corruption_run_audit(civ_corruption_engine_t *engine,
                                      civ_float_t intensity);
bool civ_corruption_is_npc_compromised(const civ_corruption_engine_t *engine,
                                       const char *npc_id);

#endif /* CIVILIZATION_CORRUPTION_H */
