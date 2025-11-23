/**
 * @file system_orchestrator.c
 * @brief Implementation of system orchestrator
 */

#include "../../../include/core/simulation_engine/system_orchestrator.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_system_orchestrator_t* civ_system_orchestrator_create(void) {
    civ_system_orchestrator_t* so = (civ_system_orchestrator_t*)CIV_MALLOC(sizeof(civ_system_orchestrator_t));
    if (!so) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate system orchestrator");
        return NULL;
    }
    
    civ_system_orchestrator_init(so);
    return so;
}

void civ_system_orchestrator_destroy(civ_system_orchestrator_t* so) {
    if (!so) return;
    
    CIV_FREE(so->systems);
    if (so->execution_order) {
        for (size_t i = 0; i < so->order_count; i++) {
            CIV_FREE(so->execution_order[i]);
        }
        CIV_FREE(so->execution_order);
    }
    CIV_FREE(so);
}

void civ_system_orchestrator_init(civ_system_orchestrator_t* so) {
    if (!so) return;
    
    memset(so, 0, sizeof(civ_system_orchestrator_t));
    so->system_capacity = 32;
    so->systems = (civ_updatable_t*)CIV_CALLOC(so->system_capacity, sizeof(civ_updatable_t));
    so->parallel_execution = false;
    so->max_workers = 4;
}

civ_result_t civ_system_orchestrator_register(civ_system_orchestrator_t* so, 
                                              const char* name, civ_updatable_t* updatable,
                                              const char** dependencies, size_t dep_count) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!so || !name || !updatable) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Check if already registered */
    for (size_t i = 0; i < so->system_count; i++) {
        if (so->systems[i].get_name && strcmp(so->systems[i].get_name(so->systems[i].system), name) == 0) {
            result.error = CIV_ERROR_INVALID_STATE;
            result.message = "System already registered";
            return result;
        }
    }
    
    /* Expand capacity if needed */
    if (so->system_count >= so->system_capacity) {
        so->system_capacity *= 2;
        so->systems = (civ_updatable_t*)CIV_REALLOC(so->systems, 
                                                    so->system_capacity * sizeof(civ_updatable_t));
    }
    
    /* Register system */
    so->systems[so->system_count++] = *updatable;
    
    /* Recalculate execution order */
    civ_system_orchestrator_calculate_order(so);
    
    civ_log(CIV_LOG_INFO, "Registered system: %s", name);
    
    return result;
}

void civ_system_orchestrator_unregister(civ_system_orchestrator_t* so, const char* name) {
    if (!so || !name) return;
    
    for (size_t i = 0; i < so->system_count; i++) {
        if (so->systems[i].get_name && 
            strcmp(so->systems[i].get_name(so->systems[i].system), name) == 0) {
            /* Remove system */
            memmove(&so->systems[i], &so->systems[i + 1], 
                    (so->system_count - i - 1) * sizeof(civ_updatable_t));
            so->system_count--;
            
            /* Recalculate order */
            civ_system_orchestrator_calculate_order(so);
            
            civ_log(CIV_LOG_INFO, "Unregistered system: %s", name);
            return;
        }
    }
}

civ_result_t civ_system_orchestrator_calculate_order(civ_system_orchestrator_t* so) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!so) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Simple topological sort for dependency ordering */
    /* In production, would use proper topological sort algorithm */
    
    /* Free old order */
    if (so->execution_order) {
        for (size_t i = 0; i < so->order_count; i++) {
            CIV_FREE(so->execution_order[i]);
        }
        CIV_FREE(so->execution_order);
    }
    
    /* Create new order (simplified - just use registration order) */
    so->order_count = so->system_count;
    so->execution_order = (char**)CIV_CALLOC(so->order_count, sizeof(char*));
    
    if (so->execution_order) {
        for (size_t i = 0; i < so->system_count; i++) {
            if (so->systems[i].get_name) {
                const char* name = so->systems[i].get_name(so->systems[i].system);
                size_t name_len = strlen(name) + 1;
                so->execution_order[i] = (char*)CIV_MALLOC(name_len);
                if (so->execution_order[i]) {
                    strcpy(so->execution_order[i], name);
                }
            }
        }
    }
    
    return result;
}

civ_result_t civ_system_orchestrator_update_all(civ_system_orchestrator_t* so, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!so) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update systems in order */
    for (size_t i = 0; i < so->system_count; i++) {
        civ_updatable_t* updatable = &so->systems[i];
        
        if (!updatable->is_enabled || !updatable->is_enabled(updatable->system)) {
            continue;
        }
        
        if (updatable->update) {
            clock_t start = clock();
            civ_result_t update_result = updatable->update(updatable->system, time_delta);
            clock_t end = clock();
            
            civ_float_t update_time = ((civ_float_t)(end - start)) / CLOCKS_PER_SEC * 1000.0f;
            
            if (CIV_FAILED(update_result)) {
                civ_log(CIV_LOG_WARNING, "System update failed: %s", update_result.message);
            }
        }
    }
    
    return result;
}

void civ_system_orchestrator_enable_system(civ_system_orchestrator_t* so, const char* name, bool enabled) {
    if (!so || !name) return;
    
    for (size_t i = 0; i < so->system_count; i++) {
        if (so->systems[i].get_name && 
            strcmp(so->systems[i].get_name(so->systems[i].system), name) == 0) {
            if (so->systems[i].set_enabled) {
                so->systems[i].set_enabled(so->systems[i].system, enabled);
            }
            return;
        }
    }
}

civ_system_status_t* civ_system_orchestrator_get_status(civ_system_orchestrator_t* so, const char* name) {
    if (!so || !name) return NULL;
    
    /* Find system and return status */
    /* Simplified - would need to track status separately */
    return NULL;
}

civ_float_t civ_system_orchestrator_get_overall_health(const civ_system_orchestrator_t* so) {
    if (!so || so->system_count == 0) return 1.0f;
    
    /* Calculate average health of all systems */
    /* Simplified - would track health per system */
    return 1.0f;
}

