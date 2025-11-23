/**
 * @file state_persistence.h
 * @brief State persistence for save/load functionality
 */

#ifndef CIVILIZATION_STATE_PERSISTENCE_H
#define CIVILIZATION_STATE_PERSISTENCE_H

#include "../../common.h"
#include "../../types.h"

/* State persistence manager */
typedef struct {
    char* save_directory;
    bool compression_enabled;
    bool encryption_enabled;
} civ_state_persistence_t;

/* Function declarations */
civ_state_persistence_t* civ_state_persistence_create(const char* save_directory);
void civ_state_persistence_destroy(civ_state_persistence_t* sp);
void civ_state_persistence_init(civ_state_persistence_t* sp, const char* save_directory);

civ_result_t civ_state_persistence_save(civ_state_persistence_t* sp, const char* filename, const void* data, size_t data_size);
civ_result_t civ_state_persistence_load(civ_state_persistence_t* sp, const char* filename, void* data, size_t* data_size);
civ_result_t civ_state_persistence_list_saves(civ_state_persistence_t* sp, char** filenames, size_t* count);

#endif /* CIVILIZATION_STATE_PERSISTENCE_H */

