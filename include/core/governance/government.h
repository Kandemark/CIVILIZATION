/**
 * @file government.h
 * @brief Government and governance system
 */

#ifndef CIVILIZATION_GOVERNMENT_H
#define CIVILIZATION_GOVERNMENT_H

#include "../../common.h"
#include "../../types.h"
#include "custom_governance.h"

/* Government type enumeration */
typedef enum {
    CIV_GOV_TYPE_DEMOCRACY = 0,
    CIV_GOV_TYPE_MONARCHY,
    CIV_GOV_TYPE_TRIBAL,
    CIV_GOV_TYPE_THEOCRACY,
    CIV_GOV_TYPE_DICTATORSHIP,
    CIV_GOV_TYPE_REPUBLIC
} civ_government_type_t;

/* Government structure */
typedef struct {
    char name[STRING_MEDIUM_LEN];
    civ_government_type_t gov_type;
    civ_float_t stability;
    civ_float_t legitimacy;
    civ_float_t efficiency;
    char** subunit_ids;
    size_t subunit_count;
    size_t subunit_capacity;
    civ_float_t* decision_priorities;
    size_t priority_count;
} civ_government_t;

/* Function declarations */
civ_government_t* civ_government_create(const char* name, civ_government_type_t type);
void civ_government_destroy(civ_government_t* gov);
void civ_government_init(civ_government_t* gov, const char* name, civ_government_type_t type);

civ_float_t civ_government_collect_taxes(civ_government_t* gov);
void civ_government_add_subunit(civ_government_t* gov, const char* subunit_id);
civ_float_t civ_government_get_stability(const civ_government_t* gov);

#endif /* CIVILIZATION_GOVERNMENT_H */

