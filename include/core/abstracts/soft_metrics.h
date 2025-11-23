/**
 * @file soft_metrics.h
 * @brief Soft metrics system (happiness, legitimacy, prestige)
 */

#ifndef CIVILIZATION_SOFT_METRICS_H
#define CIVILIZATION_SOFT_METRICS_H

#include "../../common.h"
#include "../../types.h"

/* Mood enumeration */
typedef enum {
    CIV_MOOD_REBELLIOUS = 0,
    CIV_MOOD_UNHAPPY = 1,
    CIV_MOOD_DISCONTENT = 2,
    CIV_MOOD_CONTENT = 3,
    CIV_MOOD_HAPPY = 4,
    CIV_MOOD_ECSTATIC = 5
} civ_mood_t;

/* Happiness metrics structure */
typedef struct {
    civ_float_t base_happiness;
    civ_float_t stability;
    civ_float_t loyalty;
    civ_float_t* recent_changes;
    size_t change_count;
    size_t change_capacity;
} civ_happiness_metrics_t;

/* Legitimacy system structure */
typedef struct {
    civ_float_t legitimacy;
    civ_float_t political_stability;
    civ_float_t corruption_level;
    civ_float_t government_approval;
} civ_legitimacy_system_t;

/* Prestige system structure */
typedef struct {
    civ_float_t prestige;
    civ_float_t cultural_influence;
    civ_float_t technological_achievements;
    civ_float_t military_prowess;
    civ_float_t* international_relations;
    size_t relation_count;
} civ_prestige_system_t;

/* Soft metrics manager structure */
typedef struct {
    civ_happiness_metrics_t happiness_metrics;
    civ_legitimacy_system_t legitimacy_system;
    civ_prestige_system_t prestige_system;
} civ_soft_metrics_manager_t;

/* Function declarations */
civ_soft_metrics_manager_t* civ_soft_metrics_manager_create(void);
void civ_soft_metrics_manager_destroy(civ_soft_metrics_manager_t* sm);
void civ_soft_metrics_manager_init(civ_soft_metrics_manager_t* sm);

civ_float_t civ_happiness_metrics_get_overall(const civ_happiness_metrics_t* hm);
civ_mood_t civ_happiness_metrics_get_mood(const civ_happiness_metrics_t* hm);
void civ_happiness_metrics_add_change(civ_happiness_metrics_t* hm, civ_float_t change);

civ_float_t civ_legitimacy_calculate_score(const civ_legitimacy_system_t* ls);
void civ_soft_metrics_update_from_economy(civ_soft_metrics_manager_t* sm, const void* economic_data);
void civ_soft_metrics_update_from_events(civ_soft_metrics_manager_t* sm, const void* events, size_t event_count);

/* Serialization */
char* civ_soft_metrics_to_dict(const civ_soft_metrics_manager_t* sm);

#endif /* CIVILIZATION_SOFT_METRICS_H */

