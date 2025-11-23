/**
 * @file iserializable.h
 * @brief Serializable interface for save/load functionality
 */

#ifndef CIVILIZATION_ISERIALIZABLE_H
#define CIVILIZATION_ISERIALIZABLE_H

#include "../../common.h"
#include "../../types.h"

/* Serializable interface */
typedef struct civ_serializable {
    void* object;
    civ_result_t (*serialize)(const void* object, char* buffer, size_t buffer_size, size_t* written);
    civ_result_t (*deserialize)(void* object, const char* buffer, size_t buffer_size);
    size_t (*get_serialized_size)(const void* object);
} civ_serializable_t;

#endif /* CIVILIZATION_ISERIALIZABLE_H */

