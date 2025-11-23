/**
 * @file language_evolution.h
 * @brief Language evolution system
 */

#ifndef CIVILIZATION_LANGUAGE_EVOLUTION_H
#define CIVILIZATION_LANGUAGE_EVOLUTION_H

#include "../../common.h"
#include "../../types.h"
#include "writing_system.h"

/* Language - evolution-based, no predefined families */
typedef struct {
    char id[STRING_SHORT_LEN];
    char name[STRING_MEDIUM_LEN];
    char parent_id[STRING_SHORT_LEN];  /* Parent language (for evolution) */
    
    char writing_system_id[STRING_SHORT_LEN];  /* Associated writing system */
    
    civ_float_t complexity;  /* 0.0 to 1.0 */
    civ_float_t prestige;    /* 0.0 to 1.0 */
    civ_float_t speakers;    /* Number of speakers (normalized) */
    
    char* vocabulary;
    size_t vocabulary_size;
    
    time_t creation_time;
    time_t last_evolution;
} civ_language_t;

/* Language evolution system */
typedef struct {
    civ_language_t* languages;
    size_t language_count;
    size_t language_capacity;
    
    civ_float_t evolution_rate;
    civ_float_t divergence_threshold;
} civ_language_evolution_t;

/* Function declarations */
civ_language_evolution_t* civ_language_evolution_create(void);
void civ_language_evolution_destroy(civ_language_evolution_t* evolution);
void civ_language_evolution_init(civ_language_evolution_t* evolution);

civ_language_t* civ_language_create(const char* id, const char* name, const char* parent_id);
civ_language_t* civ_language_evolve_from(civ_language_evolution_t* evolution, const civ_language_t* parent, const char* new_id, const char* new_name);
void civ_language_destroy(civ_language_t* language);
civ_result_t civ_language_evolution_update(civ_language_evolution_t* evolution, civ_float_t time_delta);
civ_result_t civ_language_evolve(civ_language_t* language, civ_float_t time_delta);
civ_float_t civ_language_calculate_similarity(const civ_language_t* a, const civ_language_t* b);
civ_result_t civ_language_evolution_add(civ_language_evolution_t* evolution, civ_language_t* language);
civ_language_t* civ_language_evolution_find(const civ_language_evolution_t* evolution, const char* id);

#endif /* CIVILIZATION_LANGUAGE_EVOLUTION_H */

