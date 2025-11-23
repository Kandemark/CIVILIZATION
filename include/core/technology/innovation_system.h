/**
 * @file innovation_system.h
 * @brief Technology and innovation system
 */

#ifndef CIVILIZATION_INNOVATION_SYSTEM_H
#define CIVILIZATION_INNOVATION_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Technology node structure - evolution-based, no predefined categories/eras */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    char description[STRING_MAX_LEN];
    char category[STRING_SHORT_LEN];  /* Dynamic category name */
    civ_float_t tech_level;  /* Continuous level instead of era */
    civ_float_t base_research_cost;
    civ_float_t progress;
    bool researched;
    civ_id_t* prerequisites;
    size_t prerequisite_count;
} civ_technology_node_t;

/* Innovation system structure */
typedef struct {
    civ_float_t tech_level;
    civ_float_t research_budget;
    civ_technology_node_t* technologies;
    size_t tech_count;
    size_t tech_capacity;
    civ_id_t* researched_techs;
    size_t researched_count;
    size_t researched_capacity;
    char* current_research;
} civ_innovation_system_t;

/* Function declarations */
civ_innovation_system_t* civ_innovation_system_create(void);
void civ_innovation_system_destroy(civ_innovation_system_t* is);
void civ_innovation_system_init(civ_innovation_system_t* is);

void civ_innovation_system_update(civ_innovation_system_t* is, civ_float_t time_delta);
civ_result_t civ_innovation_system_research_tech(civ_innovation_system_t* is, const char* tech_id);
civ_float_t civ_innovation_system_get_tech_level(const civ_innovation_system_t* is);
void civ_innovation_system_set_research_budget(civ_innovation_system_t* is, civ_float_t budget);

#endif /* CIVILIZATION_INNOVATION_SYSTEM_H */

