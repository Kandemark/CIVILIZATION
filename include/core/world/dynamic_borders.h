/**
 * @file dynamic_borders.h
 * @brief Dynamic borders and territory system
 */

#ifndef CIVILIZATION_DYNAMIC_BORDERS_H
#define CIVILIZATION_DYNAMIC_BORDERS_H

#include "../../common.h"
#include "../../types.h"

/* Border type enumeration */
typedef enum {
    CIV_BORDER_TYPE_NATURAL = 0,
    CIV_BORDER_TYPE_POLITICAL,
    CIV_BORDER_TYPE_DISPUTED,
    CIV_BORDER_TYPE_MILITARIZED,
    CIV_BORDER_TYPE_OPEN,
    CIV_BORDER_TYPE_CLOSED
} civ_border_type_t;

/* Border conflict type */
typedef enum {
    CIV_BORDER_CONFLICT_TERRITORIAL_DISPUTE = 0,
    CIV_BORDER_CONFLICT_RESOURCE_DISPUTE,
    CIV_BORDER_CONFLICT_BORDER_VIOLATION,
    CIV_BORDER_CONFLICT_SMUGGLING,
    CIV_BORDER_CONFLICT_REFUGEE_CRISIS
} civ_border_conflict_type_t;

/* Border segment structure */
typedef struct {
    char id[STRING_SHORT_LEN];
    char territory_a[STRING_SHORT_LEN];
    char territory_b[STRING_SHORT_LEN];
    civ_vec2_t start_point;
    civ_vec2_t end_point;
    civ_border_type_t border_type;
    civ_float_t fortification_level;
    civ_float_t tension_level;
    time_t last_incident;
} civ_border_segment_t;

/* Border conflict structure */
typedef struct {
    char id[STRING_SHORT_LEN];
    char border_segment_id[STRING_SHORT_LEN];
    civ_border_conflict_type_t conflict_type;
    civ_float_t severity;
    char** involved_parties;
    size_t party_count;
    time_t start_date;
    civ_float_t resolution_progress;
    int32_t casualties;
    civ_float_t economic_impact;
} civ_border_conflict_t;

/* Dynamic borders system */
typedef struct {
    civ_border_segment_t* border_segments;
    size_t segment_count;
    size_t segment_capacity;
    civ_border_conflict_t* active_conflicts;
    size_t conflict_count;
    size_t conflict_capacity;
} civ_dynamic_borders_t;

/* Function declarations */
civ_dynamic_borders_t* civ_dynamic_borders_create(void);
void civ_dynamic_borders_destroy(civ_dynamic_borders_t* db);
void civ_dynamic_borders_init(civ_dynamic_borders_t* db);

civ_result_t civ_dynamic_borders_create_segment(civ_dynamic_borders_t* db,
                                                 const char* territory_a, const char* territory_b,
                                                 civ_vec2_t start, civ_vec2_t end,
                                                 civ_border_type_t type);
void civ_dynamic_borders_update(civ_dynamic_borders_t* db, civ_float_t time_delta);
civ_float_t civ_dynamic_borders_get_tension(civ_dynamic_borders_t* db, const char* territory_a,
                                            const char* territory_b);

#endif /* CIVILIZATION_DYNAMIC_BORDERS_H */

