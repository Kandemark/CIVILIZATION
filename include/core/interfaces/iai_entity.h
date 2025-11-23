/**
 * @file iai_entity.h
 * @brief AI entity interface
 */

#ifndef CIVILIZATION_IAI_ENTITY_H
#define CIVILIZATION_IAI_ENTITY_H

#include "../../common.h"
#include "../../types.h"

/* AI entity interface */
typedef struct civ_ai_entity {
    void* entity;
    civ_result_t (*think)(void* entity, civ_float_t time_delta);
    civ_result_t (*act)(void* entity);
    void (*set_goal)(void* entity, const char* goal);
    const char* (*get_goal)(const void* entity);
} civ_ai_entity_t;

#endif /* CIVILIZATION_IAI_ENTITY_H */

