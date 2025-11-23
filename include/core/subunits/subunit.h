/**
 * @file subunit.h
 * @brief Subunit system
 */

#ifndef CIVILIZATION_SUBUNIT_H
#define CIVILIZATION_SUBUNIT_H

#include "../../common.h"
#include "../../types.h"

/* Subunit type */
typedef enum {
    CIV_SUBUNIT_PROVINCE = 0,
    CIV_SUBUNIT_REGION,
    CIV_SUBUNIT_CITY,
    CIV_SUBUNIT_DISTRICT,
    CIV_SUBUNIT_OTHER
} civ_subunit_type_t;

/* Subunit */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    civ_subunit_type_t type;
    
    char parent_id[STRING_SHORT_LEN];  /* Parent subunit ID */
    char** child_ids;  /* Child subunit IDs */
    size_t child_count;
    size_t child_capacity;
    
    civ_float_t autonomy;  /* 0.0 to 1.0 */
    civ_float_t loyalty;  /* 0.0 to 1.0 */
    civ_float_t development; /* 0.0 to 1.0 */
    
    int64_t population;
    civ_float_t gdp;
    
    time_t creation_time;
} civ_subunit_t;

/* Subunit manager */
typedef struct {
    civ_subunit_t* subunits;
    size_t subunit_count;
    size_t subunit_capacity;
} civ_subunit_manager_t;

/* Function declarations */
civ_subunit_manager_t* civ_subunit_manager_create(void);
void civ_subunit_manager_destroy(civ_subunit_manager_t* manager);
void civ_subunit_manager_init(civ_subunit_manager_t* manager);

civ_subunit_t* civ_subunit_create(const char* id, const char* name, civ_subunit_type_t type);
void civ_subunit_destroy(civ_subunit_t* subunit);
civ_result_t civ_subunit_manager_add(civ_subunit_manager_t* manager, civ_subunit_t* subunit);
civ_subunit_t* civ_subunit_manager_find(const civ_subunit_manager_t* manager, const char* id);
civ_result_t civ_subunit_set_parent(civ_subunit_t* subunit, const char* parent_id);
civ_result_t civ_subunit_add_child(civ_subunit_t* subunit, const char* child_id);
civ_result_t civ_subunit_manager_update(civ_subunit_manager_t* manager, civ_float_t time_delta);

#endif /* CIVILIZATION_SUBUNIT_H */

