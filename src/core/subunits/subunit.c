/**
 * @file subunit.c
 * @brief Implementation of subunit system
 */

#include "../../../include/core/subunits/subunit.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_subunit_manager_t* civ_subunit_manager_create(void) {
    civ_subunit_manager_t* manager = (civ_subunit_manager_t*)CIV_MALLOC(sizeof(civ_subunit_manager_t));
    if (!manager) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate subunit manager");
        return NULL;
    }
    
    civ_subunit_manager_init(manager);
    return manager;
}

void civ_subunit_manager_destroy(civ_subunit_manager_t* manager) {
    if (!manager) return;
    
    for (size_t i = 0; i < manager->subunit_count; i++) {
        civ_subunit_destroy(&manager->subunits[i]);
    }
    CIV_FREE(manager->subunits);
    CIV_FREE(manager);
}

void civ_subunit_manager_init(civ_subunit_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(civ_subunit_manager_t));
    manager->subunit_capacity = 64;
    manager->subunits = (civ_subunit_t*)CIV_CALLOC(manager->subunit_capacity, sizeof(civ_subunit_t));
}

civ_subunit_t* civ_subunit_create(const char* id, const char* name, civ_subunit_type_t type) {
    if (!id || !name) return NULL;
    
    civ_subunit_t* subunit = (civ_subunit_t*)CIV_MALLOC(sizeof(civ_subunit_t));
    if (!subunit) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate subunit");
        return NULL;
    }
    
    memset(subunit, 0, sizeof(civ_subunit_t));
    strncpy(subunit->id, id, sizeof(subunit->id) - 1);
    strncpy(subunit->name, name, sizeof(subunit->name) - 1);
    subunit->type = type;
    subunit->autonomy = 0.3f;
    subunit->loyalty = 0.7f;
    subunit->development = 0.5f;
    subunit->population = 0;
    subunit->gdp = 0.0f;
    subunit->creation_time = time(NULL);
    subunit->child_capacity = 16;
    subunit->child_ids = (char**)CIV_CALLOC(subunit->child_capacity, sizeof(char*));
    
    return subunit;
}

void civ_subunit_destroy(civ_subunit_t* subunit) {
    if (!subunit) return;
    
    if (subunit->child_ids) {
        for (size_t i = 0; i < subunit->child_count; i++) {
            CIV_FREE(subunit->child_ids[i]);
        }
        CIV_FREE(subunit->child_ids);
    }
}

civ_result_t civ_subunit_manager_add(civ_subunit_manager_t* manager, civ_subunit_t* subunit) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager || !subunit) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (manager->subunit_count >= manager->subunit_capacity) {
        manager->subunit_capacity *= 2;
        manager->subunits = (civ_subunit_t*)CIV_REALLOC(manager->subunits,
                                                       manager->subunit_capacity * sizeof(civ_subunit_t));
    }
    
    if (manager->subunits) {
        manager->subunits[manager->subunit_count++] = *subunit;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_subunit_t* civ_subunit_manager_find(const civ_subunit_manager_t* manager, const char* id) {
    if (!manager || !id) return NULL;
    
    for (size_t i = 0; i < manager->subunit_count; i++) {
        if (strcmp(manager->subunits[i].id, id) == 0) {
            return (civ_subunit_t*)&manager->subunits[i];
        }
    }
    
    return NULL;
}

civ_result_t civ_subunit_set_parent(civ_subunit_t* subunit, const char* parent_id) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!subunit || !parent_id) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    strncpy(subunit->parent_id, parent_id, sizeof(subunit->parent_id) - 1);
    return result;
}

civ_result_t civ_subunit_add_child(civ_subunit_t* subunit, const char* child_id) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!subunit || !child_id) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (subunit->child_count >= subunit->child_capacity) {
        subunit->child_capacity *= 2;
        subunit->child_ids = (char**)CIV_REALLOC(subunit->child_ids,
                                                 subunit->child_capacity * sizeof(char*));
    }
    
    if (subunit->child_ids) {
        size_t len = strlen(child_id) + 1;
        subunit->child_ids[subunit->child_count] = (char*)CIV_MALLOC(len);
        if (subunit->child_ids[subunit->child_count]) {
            strcpy(subunit->child_ids[subunit->child_count], child_id);
            subunit->child_count++;
        } else {
            result.error = CIV_ERROR_OUT_OF_MEMORY;
        }
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_subunit_manager_update(civ_subunit_manager_t* manager, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update all subunits */
    for (size_t i = 0; i < manager->subunit_count; i++) {
        civ_subunit_t* subunit = &manager->subunits[i];
        
        /* Development changes based on loyalty and autonomy */
        civ_float_t dev_change = (subunit->loyalty * (1.0f - subunit->autonomy) - subunit->development) * time_delta * 0.01f;
        subunit->development = CLAMP(subunit->development + dev_change, 0.0f, 1.0f);
    }
    
    return result;
}

