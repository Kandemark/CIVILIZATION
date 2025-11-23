/**
 * @file map_view.c
 * @brief Implementation of map view system
 */

#include "../../../include/core/world/map_view.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_map_view_manager_t* civ_map_view_manager_create(civ_map_t* base_map) {
    civ_map_view_manager_t* manager = (civ_map_view_manager_t*)CIV_MALLOC(sizeof(civ_map_view_manager_t));
    if (!manager) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate map view manager");
        return NULL;
    }
    
    civ_map_view_manager_init(manager, base_map);
    return manager;
}

void civ_map_view_manager_destroy(civ_map_view_manager_t* manager) {
    if (!manager) return;
    
    for (size_t i = 0; i < manager->view_count; i++) {
        CIV_FREE(manager->views[i].data);
    }
    CIV_FREE(manager->views);
    CIV_FREE(manager);
}

void civ_map_view_manager_init(civ_map_view_manager_t* manager, civ_map_t* base_map) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(civ_map_view_manager_t));
    manager->base_map = base_map;
    manager->view_count = CIV_MAP_VIEW_COUNT;
    manager->views = (civ_map_view_t*)CIV_CALLOC(manager->view_count, sizeof(civ_map_view_t));
    manager->current_view = CIV_MAP_VIEW_GEOGRAPHICAL;
    
    if (manager->views && base_map) {
        size_t tile_count = (size_t)base_map->width * (size_t)base_map->height;
        
        for (size_t i = 0; i < manager->view_count; i++) {
            manager->views[i].view_type = (civ_map_view_type_t)i;
            manager->views[i].width = base_map->width;
            manager->views[i].height = base_map->height;
            manager->views[i].visible = (i == CIV_MAP_VIEW_GEOGRAPHICAL);
            manager->views[i].opacity = 1.0f;
            manager->views[i].data = (civ_float_t*)CIV_CALLOC(tile_count, sizeof(civ_float_t));
        }
    }
}

civ_result_t civ_map_view_manager_set_view(civ_map_view_manager_t* manager, civ_map_view_type_t view_type) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (view_type >= CIV_MAP_VIEW_COUNT) {
        result.error = CIV_ERROR_INVALID_ARGUMENT;
        return result;
    }
    
    /* Hide current view */
    if (manager->current_view < CIV_MAP_VIEW_COUNT) {
        manager->views[manager->current_view].visible = false;
    }
    
    /* Show new view */
    manager->current_view = view_type;
    manager->views[view_type].visible = true;
    
    /* Update view data */
    civ_map_view_manager_update_view(manager, view_type);
    
    return result;
}

civ_result_t civ_map_view_manager_update_view(civ_map_view_manager_t* manager, civ_map_view_type_t view_type) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager || !manager->base_map) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (view_type >= CIV_MAP_VIEW_COUNT) {
        result.error = CIV_ERROR_INVALID_ARGUMENT;
        return result;
    }
    
    civ_map_view_t* view = &manager->views[view_type];
    
    switch (view_type) {
        case CIV_MAP_VIEW_GEOGRAPHICAL:
            /* Use elevation and terrain */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    civ_map_tile_t* tile = civ_map_get_tile(manager->base_map, x, y);
                    if (tile) {
                        view->data[y * view->width + x] = tile->elevation;
                    }
                }
            }
            break;
            
        case CIV_MAP_VIEW_POLITICAL:
            /* Would use border/territory data - placeholder */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    view->data[y * view->width + x] = 0.0f;  /* Placeholder */
                }
            }
            break;
            
        case CIV_MAP_VIEW_DEMOGRAPHICAL:
            /* Would use population data - placeholder */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    view->data[y * view->width + x] = 0.0f;  /* Placeholder */
                }
            }
            break;
            
        case CIV_MAP_VIEW_CULTURAL:
            /* Would use cultural influence data - placeholder */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    view->data[y * view->width + x] = 0.0f;  /* Placeholder */
                }
            }
            break;
            
        case CIV_MAP_VIEW_ECONOMIC:
            /* Use resources */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    civ_map_tile_t* tile = civ_map_get_tile(manager->base_map, x, y);
                    if (tile) {
                        view->data[y * view->width + x] = tile->resources;
                    }
                }
            }
            break;
            
        default:
            /* Placeholder for other views */
            for (int32_t y = 0; y < manager->base_map->height; y++) {
                for (int32_t x = 0; x < manager->base_map->width; x++) {
                    view->data[y * view->width + x] = 0.0f;
                }
            }
            break;
    }
    
    return result;
}

civ_float_t civ_map_view_get_tile_value(const civ_map_view_manager_t* manager, civ_map_view_type_t view_type, int32_t x, int32_t y) {
    if (!manager || view_type >= CIV_MAP_VIEW_COUNT) return 0.0f;
    if (x < 0 || y < 0 || x >= manager->views[view_type].width || y >= manager->views[view_type].height) return 0.0f;
    
    return manager->views[view_type].data[y * manager->views[view_type].width + x];
}

civ_result_t civ_map_view_manager_refresh_all(civ_map_view_manager_t* manager) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    for (size_t i = 0; i < manager->view_count; i++) {
        civ_map_view_manager_update_view(manager, (civ_map_view_type_t)i);
    }
    
    return result;
}

