/**
 * @file map_view.h
 * @brief Multiple map view system (political, geographical, demographical, etc.)
 */

#ifndef CIVILIZATION_MAP_VIEW_H
#define CIVILIZATION_MAP_VIEW_H

#include "../../common.h"
#include "../../types.h"
#include "map_generator.h"

/* Map view type */
typedef enum {
    CIV_MAP_VIEW_POLITICAL = 0,    /* Show borders, nations, territories */
    CIV_MAP_VIEW_GEOGRAPHICAL,     /* Show terrain, elevation, geography */
    CIV_MAP_VIEW_DEMOGRAPHICAL,    /* Show population density, demographics */
    CIV_MAP_VIEW_CULTURAL,         /* Show cultural influence, languages */
    CIV_MAP_VIEW_ECONOMIC,         /* Show trade routes, resources, GDP */
    CIV_MAP_VIEW_MILITARY,         /* Show military presence, fortifications */
    CIV_MAP_VIEW_DIPLOMATIC,       /* Show relations, alliances */
    CIV_MAP_VIEW_COUNT
} civ_map_view_type_t;

/* Map view data */
typedef struct {
    civ_map_view_type_t view_type;
    civ_float_t* data;  /* Per-tile data for this view */
    int32_t width;
    int32_t height;
    bool visible;
    civ_float_t opacity;  /* 0.0 to 1.0 */
} civ_map_view_t;

/* Map view manager */
typedef struct {
    civ_map_view_t* views;
    size_t view_count;
    civ_map_view_type_t current_view;
    civ_map_t* base_map;
} civ_map_view_manager_t;

/* Function declarations */
civ_map_view_manager_t* civ_map_view_manager_create(civ_map_t* base_map);
void civ_map_view_manager_destroy(civ_map_view_manager_t* manager);
void civ_map_view_manager_init(civ_map_view_manager_t* manager, civ_map_t* base_map);

civ_result_t civ_map_view_manager_set_view(civ_map_view_manager_t* manager, civ_map_view_type_t view_type);
civ_result_t civ_map_view_manager_update_view(civ_map_view_manager_t* manager, civ_map_view_type_t view_type);
civ_float_t civ_map_view_get_tile_value(const civ_map_view_manager_t* manager, civ_map_view_type_t view_type, int32_t x, int32_t y);
civ_result_t civ_map_view_manager_refresh_all(civ_map_view_manager_t* manager);

#endif /* CIVILIZATION_MAP_VIEW_H */

