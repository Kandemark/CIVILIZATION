/**
 * @file government.c
 * @brief Implementation of government system
 */

#include "../../../include/core/governance/government.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

static civ_float_t get_base_stability(civ_government_type_t type) {
    switch (type) {
        case CIV_GOV_TYPE_DEMOCRACY: return 0.7f;
        case CIV_GOV_TYPE_MONARCHY: return 0.8f;
        case CIV_GOV_TYPE_TRIBAL: return 0.6f;
        case CIV_GOV_TYPE_THEOCRACY: return 0.75f;
        case CIV_GOV_TYPE_DICTATORSHIP: return 0.5f;
        case CIV_GOV_TYPE_REPUBLIC: return 0.75f;
        default: return 0.6f;
    }
}

static civ_float_t get_base_efficiency(civ_government_type_t type) {
    switch (type) {
        case CIV_GOV_TYPE_DEMOCRACY: return 0.6f;
        case CIV_GOV_TYPE_MONARCHY: return 0.7f;
        case CIV_GOV_TYPE_TRIBAL: return 0.5f;
        case CIV_GOV_TYPE_THEOCRACY: return 0.65f;
        case CIV_GOV_TYPE_DICTATORSHIP: return 0.8f;
        case CIV_GOV_TYPE_REPUBLIC: return 0.7f;
        default: return 0.6f;
    }
}

civ_government_t* civ_government_create(const char* name, civ_government_type_t type) {
    civ_government_t* gov = (civ_government_t*)CIV_MALLOC(sizeof(civ_government_t));
    if (!gov) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate government");
        return NULL;
    }
    
    civ_government_init(gov, name, type);
    return gov;
}

void civ_government_destroy(civ_government_t* gov) {
    if (!gov) return;
    
    if (gov->subunit_ids) {
        for (size_t i = 0; i < gov->subunit_count; i++) {
            CIV_FREE(gov->subunit_ids[i]);
        }
        CIV_FREE(gov->subunit_ids);
    }
    
    CIV_FREE(gov->decision_priorities);
    CIV_FREE(gov);
}

void civ_government_init(civ_government_t* gov, const char* name, civ_government_type_t type) {
    if (!gov || !name) return;
    
    memset(gov, 0, sizeof(civ_government_t));
    strncpy(gov->name, name, sizeof(gov->name) - 1);
    gov->gov_type = type;
    gov->stability = get_base_stability(type);
    gov->legitimacy = 0.7f;
    gov->efficiency = get_base_efficiency(type);
    
    gov->subunit_capacity = 20;
    gov->subunit_ids = (char**)CIV_CALLOC(gov->subunit_capacity, sizeof(char*));
    
    /* Initialize decision priorities */
    gov->priority_count = 5;
    gov->decision_priorities = (civ_float_t*)CIV_CALLOC(gov->priority_count, sizeof(civ_float_t));
    if (gov->decision_priorities) {
        /* Default priorities */
        for (size_t i = 0; i < gov->priority_count; i++) {
            gov->decision_priorities[i] = 0.2f; /* Equal weighting */
        }
    }
}

civ_float_t civ_government_collect_taxes(civ_government_t* gov) {
    if (!gov) return 0.0f;
    
    /* Simplified tax collection based on subunits and efficiency */
    civ_float_t base_tax = (civ_float_t)gov->subunit_count * 100.0f;
    return base_tax * gov->efficiency;
}

void civ_government_add_subunit(civ_government_t* gov, const char* subunit_id) {
    if (!gov || !subunit_id) return;
    
    if (gov->subunit_count >= gov->subunit_capacity) {
        gov->subunit_capacity *= 2;
        gov->subunit_ids = (char**)CIV_REALLOC(gov->subunit_ids, 
                                               gov->subunit_capacity * sizeof(char*));
    }
    
    if (gov->subunit_ids) {
        size_t id_len = strlen(subunit_id) + 1;
        gov->subunit_ids[gov->subunit_count] = (char*)CIV_MALLOC(id_len);
        if (gov->subunit_ids[gov->subunit_count]) {
            strcpy(gov->subunit_ids[gov->subunit_count], subunit_id);
            gov->subunit_count++;
        }
    }
}

civ_float_t civ_government_get_stability(const civ_government_t* gov) {
    if (!gov) return 0.0f;
    return gov->stability;
}

