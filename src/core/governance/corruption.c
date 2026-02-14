/**
 * @file corruption.c
 * @brief Implementation of systemic corruption engine
 */

#include "../../../include/core/governance/corruption.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_corruption_engine_t *civ_corruption_engine_create(void) {
  civ_corruption_engine_t *engine =
      (civ_corruption_engine_t *)CIV_MALLOC(sizeof(civ_corruption_engine_t));
  if (!engine)
    return NULL;

  memset(engine, 0, sizeof(civ_corruption_engine_t));
  engine->node_capacity = 32;
  engine->nodes = (civ_corruption_node_t *)CIV_CALLOC(
      engine->node_capacity, sizeof(civ_corruption_node_t));
  engine->audit_effectiveness = 0.5f;

  return engine;
}

void civ_corruption_engine_destroy(civ_corruption_engine_t *engine) {
  if (!engine)
    return;
  CIV_FREE(engine->nodes);
  CIV_FREE(engine);
}

civ_result_t civ_corruption_add_involvement(civ_corruption_engine_t *engine,
                                            const char *npc_id,
                                            civ_float_t amount) {
  if (!engine || !npc_id)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  /* Find existing node or add new */
  for (size_t i = 0; i < engine->node_count; i++) {
    if (strcmp(engine->nodes[i].npc_id, npc_id) == 0) {
      engine->nodes[i].involvement =
          CLAMP(engine->nodes[i].involvement + amount, 0.0f, 1.0f);
      return (civ_result_t){CIV_OK, NULL};
    }
  }

  /* Add new node */
  if (engine->node_count >= engine->node_capacity) {
    engine->node_capacity *= 2;
    engine->nodes = (civ_corruption_node_t *)CIV_REALLOC(
        engine->nodes, engine->node_capacity * sizeof(civ_corruption_node_t));
  }

  if (engine->nodes) {
    civ_corruption_node_t *node = &engine->nodes[engine->node_count++];
    strncpy(node->npc_id, npc_id, STRING_SHORT_LEN - 1);
    node->involvement = CLAMP(amount, 0.0f, 1.0f);
    node->influence = 1.0f;
    node->clique_count = 0;
  } else {
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
  }

  return (civ_result_t){CIV_OK, NULL};
}

civ_float_t
civ_corruption_calculate_leakage(const civ_corruption_engine_t *engine,
                                 civ_float_t total_budget) {
  if (!engine)
    return 0.0f;

  civ_float_t leakage = 0.0f;
  for (size_t i = 0; i < engine->node_count; i++) {
    leakage +=
        engine->nodes[i].involvement * engine->nodes[i].influence * 0.01f;
  }

  return total_budget *
         CLAMP(leakage + engine->systemic_index * 0.05f, 0.0f, 0.8f);
}

civ_result_t civ_corruption_run_audit(civ_corruption_engine_t *engine,
                                      civ_float_t intensity) {
  if (!engine)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null engine"};

  /* Audits reduce involvement but may cause tension/instability */
  civ_float_t detection_power = intensity * engine->audit_effectiveness;

  for (size_t i = 0; i < engine->node_count; i++) {
    if (engine->nodes[i].involvement > (1.0f - detection_power)) {
      /* Corruption detected and suppressed */
      engine->nodes[i].involvement *= (1.0f - intensity);
      civ_log(CIV_LOG_INFO, "Audit detected corruption in node %s",
              engine->nodes[i].npc_id);
    }
  }

  return (civ_result_t){CIV_OK, NULL};
}

bool civ_corruption_is_npc_compromised(const civ_corruption_engine_t *engine,
                                       const char *npc_id) {
  if (!engine || !npc_id)
    return false;

  for (size_t i = 0; i < engine->node_count; i++) {
    if (strcmp(engine->nodes[i].npc_id, npc_id) == 0) {
      return engine->nodes[i].involvement > 0.3f;
    }
  }

  return false;
}
