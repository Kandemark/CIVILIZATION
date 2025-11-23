/**
 * @file cultural_display.h
 * @brief Visualization system for cultural processes
 */

#ifndef CIVILIZATION_CULTURAL_DISPLAY_H
#define CIVILIZATION_CULTURAL_DISPLAY_H

#include "../../common.h"
#include "../../types.h"
#include "../culture/cultural_assimilation.h"
#include "../culture/cultural_identity.h"

/* Display data for assimilation */
typedef struct {
    char source_culture[STRING_MEDIUM_LEN];
    char target_culture[STRING_MEDIUM_LEN];
    char region[STRING_MEDIUM_LEN];
    civ_assimilation_type_t type;
    civ_float_t progress;  /* 0.0 to 1.0 */
    civ_float_t adoption_level; /* 0.0 to 1.0 */
    int32_t population_affected;
    char status[STRING_SHORT_LEN];  /* "spreading", "adopting", "imposing", "complete" */
} civ_assimilation_display_t;

/* Display data for culture */
typedef struct {
    char culture_id[STRING_SHORT_LEN];
    char culture_name[STRING_MEDIUM_LEN];
    civ_float_t influence;
    civ_float_t cohesion;
    size_t trait_count;
    int32_t regions_influenced;
} civ_culture_display_t;

/* Cultural display system */
typedef struct {
    civ_assimilation_display_t* assimilation_displays;
    size_t assimilation_display_count;
    size_t assimilation_display_capacity;
    
    civ_culture_display_t* culture_displays;
    size_t culture_display_count;
    size_t culture_display_capacity;
} civ_cultural_display_t;

/* Function declarations */
civ_cultural_display_t* civ_cultural_display_create(void);
void civ_cultural_display_destroy(civ_cultural_display_t* display);
void civ_cultural_display_init(civ_cultural_display_t* display);

civ_result_t civ_cultural_display_update(civ_cultural_display_t* display,
                                        civ_assimilation_tracker_t* assimilation_tracker,
                                        civ_cultural_identity_manager_t* identity_manager);
civ_result_t civ_cultural_display_get_assimilation_info(const civ_cultural_display_t* display,
                                                        const char* source_id, const char* target_id,
                                                        civ_assimilation_display_t* out);
civ_result_t civ_cultural_display_get_culture_info(const civ_cultural_display_t* display,
                                                  const char* culture_id,
                                                  civ_culture_display_t* out);
char* civ_cultural_display_format_assimilation_report(const civ_cultural_display_t* display);

#endif /* CIVILIZATION_CULTURAL_DISPLAY_H */

