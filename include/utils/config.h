/**
 * @file config.h
 * @brief Configuration system
 */

#ifndef CIVILIZATION_CONFIG_H
#define CIVILIZATION_CONFIG_H

#include "../common.h"
#include "../types.h"

/* Configuration value types */
typedef enum {
    CIV_CONFIG_TYPE_INT = 0,
    CIV_CONFIG_TYPE_FLOAT,
    CIV_CONFIG_TYPE_BOOL,
    CIV_CONFIG_TYPE_STRING
} civ_config_type_t;

/* Configuration entry */
typedef struct {
    char key[STRING_SHORT_LEN];
    civ_config_type_t type;
    union {
        int32_t int_value;
        civ_float_t float_value;
        bool bool_value;
        char string_value[STRING_MAX_LEN];
    } value;
} civ_config_entry_t;

/* Configuration manager */
typedef struct {
    civ_config_entry_t* entries;
    size_t entry_count;
    size_t entry_capacity;
} civ_config_manager_t;

/* Function declarations */
civ_config_manager_t* civ_config_manager_create(void);
void civ_config_manager_destroy(civ_config_manager_t* cm);
void civ_config_manager_init(civ_config_manager_t* cm);

civ_result_t civ_config_set_int(civ_config_manager_t* cm, const char* key, int32_t value);
civ_result_t civ_config_set_float(civ_config_manager_t* cm, const char* key, civ_float_t value);
civ_result_t civ_config_set_bool(civ_config_manager_t* cm, const char* key, bool value);
civ_result_t civ_config_set_string(civ_config_manager_t* cm, const char* key, const char* value);

civ_result_t civ_config_get_int(const civ_config_manager_t* cm, const char* key, int32_t* out);
civ_result_t civ_config_get_float(const civ_config_manager_t* cm, const char* key, civ_float_t* out);
civ_result_t civ_config_get_bool(const civ_config_manager_t* cm, const char* key, bool* out);
civ_result_t civ_config_get_string(const civ_config_manager_t* cm, const char* key, char* out, size_t out_size);

civ_result_t civ_config_load_from_file(civ_config_manager_t* cm, const char* filename);
civ_result_t civ_config_save_to_file(const civ_config_manager_t* cm, const char* filename);

#endif /* CIVILIZATION_CONFIG_H */

