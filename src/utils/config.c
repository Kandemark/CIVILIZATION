/**
 * @file config.c
 * @brief Implementation of configuration system
 */

#include "../../include/utils/config.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

civ_config_manager_t* civ_config_manager_create(void) {
    civ_config_manager_t* cm = (civ_config_manager_t*)CIV_MALLOC(sizeof(civ_config_manager_t));
    if (!cm) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate config manager");
        return NULL;
    }
    
    civ_config_manager_init(cm);
    return cm;
}

void civ_config_manager_destroy(civ_config_manager_t* cm) {
    if (!cm) return;
    CIV_FREE(cm->entries);
    CIV_FREE(cm);
}

void civ_config_manager_init(civ_config_manager_t* cm) {
    if (!cm) return;
    
    memset(cm, 0, sizeof(civ_config_manager_t));
    cm->entry_capacity = 100;
    cm->entries = (civ_config_entry_t*)CIV_CALLOC(cm->entry_capacity, sizeof(civ_config_entry_t));
}

static civ_config_entry_t* find_or_create_entry(civ_config_manager_t* cm, const char* key, civ_config_type_t type) {
    if (!cm || !key) return NULL;
    
    /* Find existing entry */
    for (size_t i = 0; i < cm->entry_count; i++) {
        if (strcmp(cm->entries[i].key, key) == 0) {
            cm->entries[i].type = type;
            return &cm->entries[i];
        }
    }
    
    /* Create new entry */
    if (cm->entry_count >= cm->entry_capacity) {
        cm->entry_capacity *= 2;
        cm->entries = (civ_config_entry_t*)CIV_REALLOC(cm->entries,
                                                      cm->entry_capacity * sizeof(civ_config_entry_t));
    }
    
    if (cm->entries) {
        civ_config_entry_t* entry = &cm->entries[cm->entry_count++];
        memset(entry, 0, sizeof(civ_config_entry_t));
        strncpy(entry->key, key, sizeof(entry->key) - 1);
        entry->type = type;
        return entry;
    }
    
    return NULL;
}

civ_result_t civ_config_set_int(civ_config_manager_t* cm, const char* key, int32_t value) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_config_entry_t* entry = find_or_create_entry(cm, key, CIV_CONFIG_TYPE_INT);
    if (entry) {
        entry->value.int_value = value;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_config_set_float(civ_config_manager_t* cm, const char* key, civ_float_t value) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_config_entry_t* entry = find_or_create_entry(cm, key, CIV_CONFIG_TYPE_FLOAT);
    if (entry) {
        entry->value.float_value = value;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_config_set_bool(civ_config_manager_t* cm, const char* key, bool value) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_config_entry_t* entry = find_or_create_entry(cm, key, CIV_CONFIG_TYPE_BOOL);
    if (entry) {
        entry->value.bool_value = value;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_config_set_string(civ_config_manager_t* cm, const char* key, const char* value) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key || !value) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_config_entry_t* entry = find_or_create_entry(cm, key, CIV_CONFIG_TYPE_STRING);
    if (entry) {
        strncpy(entry->value.string_value, value, sizeof(entry->value.string_value) - 1);
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_config_get_int(const civ_config_manager_t* cm, const char* key, int32_t* out) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key || !out) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < cm->entry_count; i++) {
        if (strcmp(cm->entries[i].key, key) == 0 && cm->entries[i].type == CIV_CONFIG_TYPE_INT) {
            *out = cm->entries[i].value.int_value;
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

civ_result_t civ_config_get_float(const civ_config_manager_t* cm, const char* key, civ_float_t* out) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key || !out) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < cm->entry_count; i++) {
        if (strcmp(cm->entries[i].key, key) == 0 && cm->entries[i].type == CIV_CONFIG_TYPE_FLOAT) {
            *out = cm->entries[i].value.float_value;
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

civ_result_t civ_config_get_bool(const civ_config_manager_t* cm, const char* key, bool* out) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key || !out) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < cm->entry_count; i++) {
        if (strcmp(cm->entries[i].key, key) == 0 && cm->entries[i].type == CIV_CONFIG_TYPE_BOOL) {
            *out = cm->entries[i].value.bool_value;
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

civ_result_t civ_config_get_string(const civ_config_manager_t* cm, const char* key, char* out, size_t out_size) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !key || !out || out_size == 0) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < cm->entry_count; i++) {
        if (strcmp(cm->entries[i].key, key) == 0 && cm->entries[i].type == CIV_CONFIG_TYPE_STRING) {
            strncpy(out, cm->entries[i].value.string_value, out_size - 1);
            out[out_size - 1] = '\0';
            return result;
        }
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

civ_result_t civ_config_load_from_file(civ_config_manager_t* cm, const char* filename) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !filename) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        result.error = CIV_ERROR_IO;
        result.message = "Failed to open config file";
        return result;
    }
    
    /* Simple config file parsing (key=value format) */
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\n') continue;
        
        /* Parse key=value */
        char* eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char* key = line;
            char* value = eq + 1;
            
            /* Trim whitespace */
            while (*value == ' ' || *value == '\t') value++;
            size_t len = strlen(value);
            while (len > 0 && (value[len-1] == ' ' || value[len-1] == '\t' || value[len-1] == '\n')) {
                value[--len] = '\0';
            }
            
            /* Try to determine type and set value */
            if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) {
                civ_config_set_bool(cm, key, strcmp(value, "true") == 0);
            } else if (strchr(value, '.') != NULL) {
                civ_float_t fval = (civ_float_t)atof(value);
                civ_config_set_float(cm, key, fval);
            } else {
                int32_t ival = (int32_t)atoi(value);
                civ_config_set_int(cm, key, ival);
            }
        }
    }
    
    fclose(file);
    return result;
}

civ_result_t civ_config_save_to_file(const civ_config_manager_t* cm, const char* filename) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cm || !filename) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        result.error = CIV_ERROR_IO;
        result.message = "Failed to open config file for writing";
        return result;
    }
    
    fprintf(file, "# Civilization Configuration File\n");
    fprintf(file, "# Generated automatically\n\n");
    
    for (size_t i = 0; i < cm->entry_count; i++) {
        const civ_config_entry_t* entry = &cm->entries[i];
        
        switch (entry->type) {
            case CIV_CONFIG_TYPE_INT:
                fprintf(file, "%s=%d\n", entry->key, entry->value.int_value);
                break;
            case CIV_CONFIG_TYPE_FLOAT:
                fprintf(file, "%s=%.6f\n", entry->key, entry->value.float_value);
                break;
            case CIV_CONFIG_TYPE_BOOL:
                fprintf(file, "%s=%s\n", entry->key, entry->value.bool_value ? "true" : "false");
                break;
            case CIV_CONFIG_TYPE_STRING:
                fprintf(file, "%s=%s\n", entry->key, entry->value.string_value);
                break;
        }
    }
    
    fclose(file);
    return result;
}

