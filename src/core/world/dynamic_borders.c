/**
 * @file dynamic_borders.c
 * @brief Implementation of dynamic borders system
 */

#include "../../../include/core/world/dynamic_borders.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_dynamic_borders_t* civ_dynamic_borders_create(void) {
    civ_dynamic_borders_t* db = (civ_dynamic_borders_t*)CIV_MALLOC(sizeof(civ_dynamic_borders_t));
    if (!db) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate dynamic borders");
        return NULL;
    }
    
    civ_dynamic_borders_init(db);
    return db;
}

void civ_dynamic_borders_destroy(civ_dynamic_borders_t* db) {
    if (!db) return;
    
    if (db->border_segments) {
        CIV_FREE(db->border_segments);
    }
    
    if (db->active_conflicts) {
        for (size_t i = 0; i < db->conflict_count; i++) {
            if (db->active_conflicts[i].involved_parties) {
                for (size_t j = 0; j < db->active_conflicts[i].party_count; j++) {
                    CIV_FREE(db->active_conflicts[i].involved_parties[j]);
                }
                CIV_FREE(db->active_conflicts[i].involved_parties);
            }
        }
        CIV_FREE(db->active_conflicts);
    }
    
    CIV_FREE(db);
}

void civ_dynamic_borders_init(civ_dynamic_borders_t* db) {
    if (!db) return;
    
    memset(db, 0, sizeof(civ_dynamic_borders_t));
    db->segment_capacity = 100;
    db->border_segments = (civ_border_segment_t*)CIV_CALLOC(db->segment_capacity, sizeof(civ_border_segment_t));
    db->conflict_capacity = 50;
    db->active_conflicts = (civ_border_conflict_t*)CIV_CALLOC(db->conflict_capacity, sizeof(civ_border_conflict_t));
}

civ_result_t civ_dynamic_borders_create_segment(civ_dynamic_borders_t* db,
                                                 const char* territory_a, const char* territory_b,
                                                 civ_vec2_t start, civ_vec2_t end,
                                                 civ_border_type_t type) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!db || !territory_a || !territory_b) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (db->segment_count >= db->segment_capacity) {
        db->segment_capacity *= 2;
        db->border_segments = (civ_border_segment_t*)CIV_REALLOC(db->border_segments,
                                                                  db->segment_capacity * sizeof(civ_border_segment_t));
    }
    
    civ_border_segment_t* segment = &db->border_segments[db->segment_count++];
    memset(segment, 0, sizeof(civ_border_segment_t));
    
    snprintf(segment->id, sizeof(segment->id), "border_%zu", db->segment_count);
    strncpy(segment->territory_a, territory_a, sizeof(segment->territory_a) - 1);
    strncpy(segment->territory_b, territory_b, sizeof(segment->territory_b) - 1);
    segment->start_point = start;
    segment->end_point = end;
    segment->border_type = type;
    segment->fortification_level = 0.0f;
    segment->tension_level = 0.0f;
    segment->last_incident = 0;
    
    return result;
}

void civ_dynamic_borders_update(civ_dynamic_borders_t* db, civ_float_t time_delta) {
    if (!db) return;
    
    /* Update tension levels for all border segments */
    for (size_t i = 0; i < db->segment_count; i++) {
        civ_border_segment_t* segment = &db->border_segments[i];
        
        /* Gradual tension decay */
        segment->tension_level = MAX(0.0f, segment->tension_level - 0.01f * time_delta);
        
        /* Border type affects tension */
        if (segment->border_type == CIV_BORDER_TYPE_DISPUTED) {
            segment->tension_level = MIN(1.0f, segment->tension_level + 0.05f * time_delta);
        } else if (segment->border_type == CIV_BORDER_TYPE_NATURAL) {
            segment->tension_level = MAX(0.0f, segment->tension_level - 0.02f * time_delta);
        }
    }
}

civ_float_t civ_dynamic_borders_get_tension(civ_dynamic_borders_t* db, const char* territory_a,
                                           const char* territory_b) {
    if (!db || !territory_a || !territory_b) return 0.0f;
    
    for (size_t i = 0; i < db->segment_count; i++) {
        civ_border_segment_t* segment = &db->border_segments[i];
        if ((strcmp(segment->territory_a, territory_a) == 0 && 
             strcmp(segment->territory_b, territory_b) == 0) ||
            (strcmp(segment->territory_a, territory_b) == 0 && 
             strcmp(segment->territory_b, territory_a) == 0)) {
            return segment->tension_level;
        }
    }
    
    return 0.0f;
}

