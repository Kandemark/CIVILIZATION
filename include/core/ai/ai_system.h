/**
 * @file ai_system.h
 * @brief Main AI system header
 */

#ifndef CIVILIZATION_AI_SYSTEM_H
#define CIVILIZATION_AI_SYSTEM_H

#include "../../common.h"
#include "../../types.h"
#include "base_ai.h"
#include "strategic_ai.h"
#include "tactical_ai.h"

/* AI system */
typedef struct {
    civ_strategic_ai_t** strategic_ais;
    civ_tactical_ai_t** tactical_ais;
    size_t strategic_count;
    size_t tactical_count;
    size_t strategic_capacity;
    size_t tactical_capacity;
} civ_ai_system_t;

/* Function declarations */
civ_ai_system_t* civ_ai_system_create(void);
void civ_ai_system_destroy(civ_ai_system_t* ai_system);
void civ_ai_system_init(civ_ai_system_t* ai_system);

civ_result_t civ_ai_system_update(civ_ai_system_t* ai_system, civ_float_t time_delta);
civ_result_t civ_ai_system_add_strategic(civ_ai_system_t* ai_system, civ_strategic_ai_t* ai);
civ_result_t civ_ai_system_add_tactical(civ_ai_system_t* ai_system, civ_tactical_ai_t* ai);

#endif /* CIVILIZATION_AI_SYSTEM_H */

