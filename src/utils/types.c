/**
 * @file types.c
 * @brief Implementation of type utilities
 */

#include "../../include/types.h"
#include "../../include/common.h"
#include <string.h>

civ_string_t* civ_string_create(const char* str) {
    if (!str) return NULL;
    
    size_t len = strlen(str);
    civ_string_t* s = (civ_string_t*)CIV_MALLOC(sizeof(civ_string_t));
    if (!s) return NULL;
    
    s->capacity = len + 1;
    s->data = (char*)CIV_MALLOC(s->capacity);
    if (!s->data) {
        CIV_FREE(s);
        return NULL;
    }
    
    strcpy(s->data, str);
    s->length = len;
    return s;
}

void civ_string_destroy(civ_string_t* str) {
    if (!str) return;
    CIV_FREE(str->data);
    CIV_FREE(str);
}

civ_string_t* civ_string_copy(const civ_string_t* src) {
    if (!src) return NULL;
    return civ_string_create(src->data);
}

int civ_string_append(civ_string_t* str, const char* append) {
    if (!str || !append) return -1;
    
    size_t append_len = strlen(append);
    size_t new_len = str->length + append_len;
    
    if (new_len >= str->capacity) {
        size_t new_capacity = (new_len + 1) * 2;
        char* new_data = (char*)CIV_REALLOC(str->data, new_capacity);
        if (!new_data) return -1;
        str->data = new_data;
        str->capacity = new_capacity;
    }
    
    strcpy(str->data + str->length, append);
    str->length = new_len;
    return 0;
}

const char* civ_string_cstr(const civ_string_t* str) {
    if (!str) return NULL;
    return str->data;
}

