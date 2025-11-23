/**
 * @file politics.h
 * @brief Main politics system header
 */

#ifndef CIVILIZATION_POLITICS_H
#define CIVILIZATION_POLITICS_H

#include "../../common.h"
#include "../../types.h"
#include "faction_system.h"

/* Politics system */
typedef struct {
    civ_faction_system_t* faction_system;
    civ_float_t legitimacy;  /* 0.0 to 1.0 */
    civ_float_t stability;   /* 0.0 to 1.0 */
} civ_politics_system_t;

/* Function declarations */
civ_politics_system_t* civ_politics_system_create(void);
void civ_politics_system_destroy(civ_politics_system_t* politics);
void civ_politics_system_init(civ_politics_system_t* politics);

civ_result_t civ_politics_system_update(civ_politics_system_t* politics, civ_float_t time_delta);
void civ_politics_system_set_legitimacy(civ_politics_system_t* politics, civ_float_t legitimacy);
civ_float_t civ_politics_system_get_stability(const civ_politics_system_t* politics);

#endif /* CIVILIZATION_POLITICS_H */

