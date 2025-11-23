/**
 * @file system_orchestrator.h
 * @brief System orchestrator for coordinated updates
 */

#ifndef CIVILIZATION_SYSTEM_ORCHESTRATOR_H
#define CIVILIZATION_SYSTEM_ORCHESTRATOR_H

#include "../../common.h"
#include "../../types.h"
#include "../interfaces/iupdatable.h"

/* System status structure */
typedef struct {
    char name[STRING_SHORT_LEN];
    bool enabled;
    civ_float_t health;  /* 0.0 to 1.0 */
    civ_float_t last_update_time;
    uint64_t update_count;
    civ_float_t avg_update_time;
} civ_system_status_t;

/* System orchestrator */
typedef struct {
    civ_updatable_t* systems;
    size_t system_count;
    size_t system_capacity;
    char** execution_order;
    size_t order_count;
    bool parallel_execution;
    uint32_t max_workers;
} civ_system_orchestrator_t;

/* Function declarations */
civ_system_orchestrator_t* civ_system_orchestrator_create(void);
void civ_system_orchestrator_destroy(civ_system_orchestrator_t* so);
void civ_system_orchestrator_init(civ_system_orchestrator_t* so);

civ_result_t civ_system_orchestrator_register(civ_system_orchestrator_t* so, 
                                               const char* name, civ_updatable_t* updatable,
                                               const char** dependencies, size_t dep_count);
void civ_system_orchestrator_unregister(civ_system_orchestrator_t* so, const char* name);
civ_result_t civ_system_orchestrator_calculate_order(civ_system_orchestrator_t* so);
civ_result_t civ_system_orchestrator_update_all(civ_system_orchestrator_t* so, civ_float_t time_delta);
void civ_system_orchestrator_enable_system(civ_system_orchestrator_t* so, const char* name, bool enabled);
civ_system_status_t* civ_system_orchestrator_get_status(civ_system_orchestrator_t* so, const char* name);
civ_float_t civ_system_orchestrator_get_overall_health(const civ_system_orchestrator_t* so);

#endif /* CIVILIZATION_SYSTEM_ORCHESTRATOR_H */

