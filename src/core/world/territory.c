/**
 * @file territory.c
 * @brief Implementation of dynamic territory system
 */

#include "../../../include/core/world/territory.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

civ_territory_manager_t* civ_territory_manager_create(void) {
    civ_territory_manager_t* manager = (civ_territory_manager_t*)CIV_MALLOC(sizeof(civ_territory_manager_t));
    if (!manager) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate territory manager");
        return NULL;
    }
    
    civ_territory_manager_init(manager);
    return manager;
}

void civ_territory_manager_destroy(civ_territory_manager_t* manager) {
    if (!manager) return;
    
    for (size_t i = 0; i < manager->region_count; i++) {
        civ_territory_region_destroy(&manager->regions[i]);
    }
    CIV_FREE(manager->regions);
    CIV_FREE(manager);
}

void civ_territory_manager_init(civ_territory_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(civ_territory_manager_t));
    manager->region_capacity = 32;
    manager->regions = (civ_territory_region_t*)CIV_CALLOC(manager->region_capacity, sizeof(civ_territory_region_t));
}

civ_territory_region_t* civ_territory_region_create(const char* nation_id) {
    if (!nation_id) return NULL;
    
    civ_territory_region_t* region = (civ_territory_region_t*)CIV_MALLOC(sizeof(civ_territory_region_t));
    if (!region) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate territory region");
        return NULL;
    }
    
    memset(region, 0, sizeof(civ_territory_region_t));
    strncpy(region->nation_id, nation_id, sizeof(region->nation_id) - 1);
    region->control_strength = 1.0f;
    region->acquisition_time = time(NULL);
    region->point_capacity = 32;
    region->boundary_points = (civ_territory_point_t*)CIV_CALLOC(region->point_capacity, sizeof(civ_territory_point_t));
    
    return region;
}

void civ_territory_region_destroy(civ_territory_region_t* region) {
    if (!region) return;
    CIV_FREE(region->boundary_points);
}

civ_result_t civ_territory_region_add_point(civ_territory_region_t* region, civ_float_t x, civ_float_t y) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!region) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (region->point_count >= region->point_capacity) {
        region->point_capacity *= 2;
        region->boundary_points = (civ_territory_point_t*)CIV_REALLOC(region->boundary_points,
                                                                     region->point_capacity * sizeof(civ_territory_point_t));
    }
    
    if (region->boundary_points) {
        region->boundary_points[region->point_count].x = x;
        region->boundary_points[region->point_count].y = y;
        region->point_count++;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_result_t civ_territory_region_calculate_area(civ_territory_region_t* region) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!region || region->point_count < 3) {
        result.error = CIV_ERROR_INVALID_ARGUMENT;
        return result;
    }
    
    /* Shoelace formula for polygon area */
    civ_float_t area = 0.0f;
    for (size_t i = 0; i < region->point_count; i++) {
        size_t j = (i + 1) % region->point_count;
        area += region->boundary_points[i].x * region->boundary_points[j].y;
        area -= region->boundary_points[j].x * region->boundary_points[i].y;
    }
    region->area = fabsf(area) / 2.0f;
    
    return result;
}

civ_result_t civ_territory_region_calculate_centroid(civ_territory_region_t* region) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!region || region->point_count == 0) {
        result.error = CIV_ERROR_INVALID_ARGUMENT;
        return result;
    }
    
    /* Calculate centroid of polygon */
    civ_float_t cx = 0.0f, cy = 0.0f;
    for (size_t i = 0; i < region->point_count; i++) {
        cx += region->boundary_points[i].x;
        cy += region->boundary_points[i].y;
    }
    
    region->centroid.latitude = cx / (civ_float_t)region->point_count;
    region->centroid.longitude = cy / (civ_float_t)region->point_count;
    
    return result;
}

bool civ_territory_region_contains_point(const civ_territory_region_t* region, civ_float_t x, civ_float_t y) {
    if (!region || region->point_count < 3) return false;
    
    /* Point-in-polygon test using ray casting */
    bool inside = false;
    for (size_t i = 0, j = region->point_count - 1; i < region->point_count; j = i++) {
        if (((region->boundary_points[i].y > y) != (region->boundary_points[j].y > y)) &&
            (x < (region->boundary_points[j].x - region->boundary_points[i].x) * 
                 (y - region->boundary_points[i].y) / 
                 (region->boundary_points[j].y - region->boundary_points[i].y) + 
                 region->boundary_points[i].x)) {
            inside = !inside;
        }
    }
    
    return inside;
}

civ_result_t civ_territory_manager_add_region(civ_territory_manager_t* manager, civ_territory_region_t* region) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager || !region) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    if (manager->region_count >= manager->region_capacity) {
        manager->region_capacity *= 2;
        manager->regions = (civ_territory_region_t*)CIV_REALLOC(manager->regions,
                                                                manager->region_capacity * sizeof(civ_territory_region_t));
    }
    
    if (manager->regions) {
        /* Calculate area and centroid */
        civ_territory_region_calculate_area(region);
        civ_territory_region_calculate_centroid(region);
        
        manager->regions[manager->region_count++] = *region;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

civ_territory_region_t* civ_territory_manager_find_region_at(civ_territory_manager_t* manager, civ_float_t x, civ_float_t y) {
    if (!manager) return NULL;
    
    /* Find region containing this point */
    for (size_t i = 0; i < manager->region_count; i++) {
        if (civ_territory_region_contains_point(&manager->regions[i], x, y)) {
            return &manager->regions[i];
        }
    }
    
    return NULL;
}

civ_result_t civ_territory_manager_update(civ_territory_manager_t* manager, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!manager) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update control strength based on time and other factors */
    for (size_t i = 0; i < manager->region_count; i++) {
        civ_territory_region_t* region = &manager->regions[i];
        
        /* Control strength may decrease over time if not maintained */
        /* This would be affected by population, military presence, etc. */
        region->control_strength = CLAMP(region->control_strength, 0.0f, 1.0f);
    }
    
    return result;
}

