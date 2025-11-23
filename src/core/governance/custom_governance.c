/**
 * @file custom_governance.c
 * @brief Implementation of custom governance
 */

#include "../../../include/core/governance/custom_governance.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_custom_governance_manager_t* civ_custom_governance_manager_create(void) {
    civ_custom_governance_manager_t* manager = (civ_custom_governance_manager_t*)CIV_MALLOC(sizeof(civ_custom_governance_manager_t));
    if (!manager) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate custom governance manager");
        return NULL;
    }
    
    civ_custom_governance_manager_init(manager);
    return manager;
}

void civ_custom_governance_manager_destroy(civ_custom_governance_manager_t* manager) {
    if (!manager) return;
    
    for (size_t i = 0; i < manager->government_count; i++) {
        civ_custom_governance_destroy(&manager->governments[i]);
    }
    CIV_FREE(manager->governments);
    CIV_FREE(manager);
}

void civ_custom_governance_manager_init(civ_custom_governance_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(civ_custom_governance_manager_t));
    manager->government_capacity = 32;
    manager->governments = (civ_custom_governance_t*)CIV_CALLOC(manager->government_capacity, sizeof(civ_custom_governance_t));
}

civ_custom_governance_t* civ_custom_governance_create(const char* id, const char* name) {
    if (!id || !name) return NULL;
    
    civ_custom_governance_t* gov = (civ_custom_governance_t*)CIV_MALLOC(sizeof(civ_custom_governance_t));
    if (!gov) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate custom governance");
        return NULL;
    }
    
    memset(gov, 0, sizeof(civ_custom_governance_t));
    strncpy(gov->id, id, sizeof(gov->id) - 1);
    strncpy(gov->name, name, sizeof(gov->name) - 1);
    gov->centralization = 0.5f;
    gov->democracy_level = 0.3f;
    gov->stability = 0.7f;
    gov->efficiency = 0.5f;
    gov->creation_time = time(NULL);
    gov->last_reform = gov->creation_time;
    gov->role_capacity = 16;
    gov->roles = (civ_governance_role_t*)CIV_CALLOC(gov->role_capacity, sizeof(civ_governance_role_t));
    
    return gov;
}

void civ_custom_governance_destroy(civ_custom_governance_t* gov) {
    if (!gov) return;
    CIV_FREE(gov->roles);
    CIV_FREE(gov->custom_rules);
}

civ_result_t civ_custom_governance_add_role(civ_custom_governance_t* gov, const char* role_name, const char* description, civ_float_t authority) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!gov || !role_name) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    authority = CLAMP(authority, 0.0f, 1.0f);
    
    if (gov->role_count >= gov->role_capacity) {
        gov->role_capacity *= 2;
        gov->roles = (civ_governance_role_t*)CIV_REALLOC(gov->roles,
                                                         gov->role_capacity * sizeof(civ_governance_role_t));
    }
    
    if (gov->roles) {
        civ_governance_role_t* role = &gov->roles[gov->role_count++];
        strncpy(role->role_name, role_name, sizeof(role->role_name) - 1);
        if (description) {
            strncpy(role->description, description, sizeof(role->description) - 1);
        }
        role->authority = authority;
        role->responsibility = authority * 0.8f;
        role->holder_count = 1;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_custom_governance_reform(civ_custom_governance_t* gov, const char* reform_description) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!gov || !reform_description) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    size_t desc_len = strlen(reform_description);
    if (gov->custom_rules) {
        size_t new_size = gov->custom_rules_size + desc_len + 2;
        gov->custom_rules = (char*)CIV_REALLOC(gov->custom_rules, new_size);
    } else {
        gov->custom_rules = (char*)CIV_MALLOC(desc_len + 1);
        gov->custom_rules_size = 0;
    }
    
    if (gov->custom_rules) {
        if (gov->custom_rules_size > 0) {
            strcat(gov->custom_rules, "\n");
        }
        strcat(gov->custom_rules, reform_description);
        gov->custom_rules_size += desc_len + 1;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        return result;
    }
    
    gov->last_reform = time(NULL);
    
    /* Reforms may affect stability */
    gov->stability = CLAMP(gov->stability - 0.1f, 0.0f, 1.0f);
    
    return result;
}

civ_result_t civ_custom_governance_update(civ_custom_governance_t* gov, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!gov) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Efficiency based on centralization and role distribution */
    civ_float_t role_efficiency = 0.0f;
    if (gov->role_count > 0) {
        for (size_t i = 0; i < gov->role_count; i++) {
            role_efficiency += gov->roles[i].responsibility * gov->roles[i].authority;
        }
        role_efficiency /= (civ_float_t)gov->role_count;
    }
    
    gov->efficiency = gov->centralization * 0.6f + role_efficiency * 0.4f;
    
    /* Stability recovers over time */
    civ_float_t stability_recovery = (1.0f - gov->stability) * time_delta * 0.01f;
    gov->stability = CLAMP(gov->stability + stability_recovery, 0.0f, 1.0f);
    
    return result;
}

civ_result_t civ_custom_governance_manager_add(civ_custom_governance_manager_t* manager, civ_custom_governance_t* gov) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager || !gov) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (manager->government_count >= manager->government_capacity) {
        manager->government_capacity *= 2;
        manager->governments = (civ_custom_governance_t*)CIV_REALLOC(manager->governments,
                                                                    manager->government_capacity * sizeof(civ_custom_governance_t));
    }
    
    if (manager->governments) {
        manager->governments[manager->government_count++] = *gov;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_custom_governance_t* civ_custom_governance_manager_find(const civ_custom_governance_manager_t* manager, const char* id) {
    if (!manager || !id) return NULL;
    
    for (size_t i = 0; i < manager->government_count; i++) {
        if (strcmp(manager->governments[i].id, id) == 0) {
            return (civ_custom_governance_t*)&manager->governments[i];
        }
    }
    
    return NULL;
}

