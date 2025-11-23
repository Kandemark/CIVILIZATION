/**
 * @file types.h
 * @brief Core type definitions for the Civilization simulation
 */

#ifndef CIVILIZATION_TYPES_H
#define CIVILIZATION_TYPES_H

#include "common.h"

/* ID types */
typedef uint32_t civ_id_t;
typedef uint64_t civ_entity_id_t;

/* Numeric types */
typedef double civ_float_t;
typedef int32_t civ_int_t;
typedef uint32_t civ_uint_t;

/* String type */
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} civ_string_t;

/* Vector2D for positions */
typedef struct {
    civ_float_t x;
    civ_float_t y;
} civ_vec2_t;

/* Vector3D for 3D positions */
typedef struct {
    civ_float_t x;
    civ_float_t y;
    civ_float_t z;
} civ_vec3_t;

/* Color RGBA */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} civ_color_t;

/* Time stamp */
typedef struct {
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
} civ_timestamp_t;

/* String functions */
civ_string_t* civ_string_create(const char* str);
void civ_string_destroy(civ_string_t* str);
civ_string_t* civ_string_copy(const civ_string_t* src);
int civ_string_append(civ_string_t* str, const char* append);
const char* civ_string_cstr(const civ_string_t* str);

#endif /* CIVILIZATION_TYPES_H */

