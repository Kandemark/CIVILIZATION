/**
 * @file iupdatable.h
 * @brief Interface for updatable systems
 */

#ifndef CIVILIZATION_IUPDATABLE_H
#define CIVILIZATION_IUPDATABLE_H

#include "../../common.h"
#include "../../types.h"

/* Updatable interface - all systems implement this */
typedef struct civ_updatable {
    void* system;  /* Pointer to the actual system */
    civ_result_t (*update)(void* system, civ_float_t time_delta);
    civ_result_t (*initialize)(void* system);
    void (*destroy)(void* system);
    const char* (*get_name)(const void* system);
    bool (*is_enabled)(const void* system);
    void (*set_enabled)(void* system, bool enabled);
} civ_updatable_t;

/* Helper function pointer type for system updates */
typedef civ_result_t (*civ_update_func_t)(void* system, civ_float_t time_delta);
typedef const char* (*civ_get_name_func_t)(const void* system);
typedef bool (*civ_is_enabled_func_t)(const void* system);
typedef void (*civ_set_enabled_func_t)(void* system, bool enabled);

/* Helper macros for implementing the interface */
#define CIV_UPDATABLE_IMPL(name, system_type) \
    static civ_result_t name##_update(void* sys, civ_float_t dt) { \
        return name##_update_impl((system_type*)sys, dt); \
    } \
    static const char* name##_get_name(const void* sys) { \
        return name##_get_name_impl((const system_type*)sys); \
    }

#endif /* CIVILIZATION_IUPDATABLE_H */

