/**
 * @file language_evolution.c
 * @brief Implementation of language evolution system
 */

#include "../../../include/core/culture/language_evolution.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_language_evolution_t* civ_language_evolution_create(void) {
    civ_language_evolution_t* evolution = (civ_language_evolution_t*)CIV_MALLOC(sizeof(civ_language_evolution_t));
    if (!evolution) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate language evolution");
        return NULL;
    }
    
    civ_language_evolution_init(evolution);
    return evolution;
}

void civ_language_evolution_destroy(civ_language_evolution_t* evolution) {
    if (!evolution) return;
    
    for (size_t i = 0; i < evolution->language_count; i++) {
        civ_language_destroy(&evolution->languages[i]);
    }
    CIV_FREE(evolution->languages);
    CIV_FREE(evolution);
}

void civ_language_evolution_init(civ_language_evolution_t* evolution) {
    if (!evolution) return;
    
    memset(evolution, 0, sizeof(civ_language_evolution_t));
    evolution->evolution_rate = 0.001f;
    evolution->divergence_threshold = 0.3f;
    evolution->language_capacity = 32;
    evolution->languages = (civ_language_t*)CIV_CALLOC(evolution->language_capacity, sizeof(civ_language_t));
}

civ_language_t* civ_language_create(const char* id, const char* name, const char* parent_id) {
    if (!id || !name) return NULL;
    
    civ_language_t* language = (civ_language_t*)CIV_MALLOC(sizeof(civ_language_t));
    if (!language) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate language");
        return NULL;
    }
    
    memset(language, 0, sizeof(civ_language_t));
    strncpy(language->id, id, sizeof(language->id) - 1);
    strncpy(language->name, name, sizeof(language->name) - 1);
    if (parent_id) {
        strncpy(language->parent_id, parent_id, sizeof(language->parent_id) - 1);
    }
    language->complexity = 0.5f;
    language->prestige = 0.5f;
    language->speakers = 1.0f;
    language->creation_time = time(NULL);
    language->last_evolution = language->creation_time;
    
    return language;
}

void civ_language_destroy(civ_language_t* language) {
    if (!language) return;
    CIV_FREE(language->vocabulary);
}

civ_result_t civ_language_evolution_update(civ_language_evolution_t* evolution, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!evolution) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Evolve all languages */
    for (size_t i = 0; i < evolution->language_count; i++) {
        civ_language_evolve(&evolution->languages[i], time_delta);
    }
    
    return result;
}

civ_result_t civ_language_evolve(civ_language_t* language, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!language) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Language complexity evolves based on speakers and prestige */
    const civ_float_t evolution_rate = 0.001f;
    civ_float_t complexity_change = (language->speakers * language->prestige - language->complexity) * 
                                    evolution_rate * time_delta;
    language->complexity = CLAMP(language->complexity + complexity_change, 0.0f, 1.0f);
    
    /* Prestige changes based on speakers and complexity */
    civ_float_t prestige_change = (language->speakers * language->complexity - language->prestige) *
                                 evolution_rate * time_delta * 0.5f;
    language->prestige = CLAMP(language->prestige + prestige_change, 0.0f, 1.0f);
    
    language->last_evolution = time(NULL);
    return result;
}

civ_float_t civ_language_calculate_similarity(const civ_language_t* a, const civ_language_t* b) {
    if (!a || !b) return 0.0f;
    
    /* Check if one is parent of the other */
    civ_float_t parent_similarity = 0.0f;
    if (strlen(a->parent_id) > 0 && strcmp(a->parent_id, b->id) == 0) {
        parent_similarity = 0.6f;  /* Child inherits from parent */
    } else if (strlen(b->parent_id) > 0 && strcmp(b->parent_id, a->id) == 0) {
        parent_similarity = 0.6f;
    } else if (strlen(a->parent_id) > 0 && strlen(b->parent_id) > 0 && strcmp(a->parent_id, b->parent_id) == 0) {
        parent_similarity = 0.4f;  /* Siblings share parent */
    }
    
    /* Complexity similarity */
    civ_float_t complexity_similarity = 1.0f - fabsf(a->complexity - b->complexity);
    
    /* Combined similarity */
    return CLAMP(parent_similarity + complexity_similarity * 0.4f, 0.0f, 1.0f);
}

/* Simple RNG for language evolution */
static uint32_t lang_rng_state = 0;
static civ_float_t lang_rng_float(void) {
    lang_rng_state = lang_rng_state * 1103515245 + 12345;
    return (civ_float_t)((lang_rng_state / 65536) % 32768) / 32768.0f;
}

civ_language_t* civ_language_evolve_from(civ_language_evolution_t* evolution, const civ_language_t* parent, const char* new_id, const char* new_name) {
    if (!evolution || !parent || !new_id || !new_name) return NULL;
    
    /* Create new language evolved from parent */
    civ_language_t* new_lang = civ_language_create(new_id, new_name, parent->id);
    if (!new_lang) return NULL;
    
    /* Inherit characteristics with variation */
    new_lang->complexity = CLAMP(parent->complexity + (lang_rng_float() - 0.5f) * 0.2f, 0.0f, 1.0f);
    new_lang->prestige = parent->prestige * 0.8f;  /* New language starts with less prestige */
    new_lang->speakers = parent->speakers * 0.1f;  /* Starts with fewer speakers */
    
    /* Add to evolution system */
    civ_language_evolution_add(evolution, new_lang);
    
    return new_lang;
}

civ_result_t civ_language_evolution_add(civ_language_evolution_t* evolution, civ_language_t* language) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!evolution || !language) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Expand if needed */
    if (evolution->language_count >= evolution->language_capacity) {
        evolution->language_capacity *= 2;
        evolution->languages = (civ_language_t*)CIV_REALLOC(evolution->languages,
                                                            evolution->language_capacity * sizeof(civ_language_t));
    }
    
    if (evolution->languages) {
        evolution->languages[evolution->language_count++] = *language;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_language_t* civ_language_evolution_find(const civ_language_evolution_t* evolution, const char* id) {
    if (!evolution || !id) return NULL;
    
    for (size_t i = 0; i < evolution->language_count; i++) {
        if (strcmp(evolution->languages[i].id, id) == 0) {
            return (civ_language_t*)&evolution->languages[i];
        }
    }
    
    return NULL;
}

