/**
 * @file land_use.h
 * @brief Land use — land value, zoning, territory development, density
 */
#ifndef CIV_ECONOMY_LAND_USE_H
#define CIV_ECONOMY_LAND_USE_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_LAND_UNDEVELOPED, CIV_LAND_AGRICULTURAL, CIV_LAND_RESIDENTIAL,
               CIV_LAND_COMMERCIAL, CIV_LAND_INDUSTRIAL, CIV_LAND_CONSERVATION,
               CIV_LAND_ZONE_COUNT } civ_land_zone_t;

typedef struct {
  civ_land_zone_t zone;
  civ_float_t     area_km2;
  civ_float_t     value_per_km2;
  civ_float_t     productivity;     /* 0.0-1.0 for this use */
  civ_float_t     development_progress; /* for converting undeveloped */
} civ_land_allocation_t;

typedef struct {
  civ_land_allocation_t allocations[CIV_LAND_ZONE_COUNT];
  civ_float_t           total_area_km2;
  civ_float_t           population_density;  /* per km2 */
  civ_float_t           land_value_index;     /* composite price index */
  civ_float_t           development_pressure; /* 0.0-1.0 */
} civ_land_use_system_t;

civ_land_use_system_t *civ_land_use_create(civ_float_t total_area_km2);
void civ_land_use_destroy(civ_land_use_system_t *l);
void civ_land_use_update(civ_land_use_system_t *l, civ_float_t time_delta,
                         int population, civ_float_t gdp_per_capita,
                         civ_float_t urbanization_rate);

void civ_land_use_rezone(civ_land_use_system_t *l, civ_land_zone_t from,
                         civ_land_zone_t to, civ_float_t amount_km2);
civ_float_t civ_land_use_value_in(const civ_land_use_system_t *l, civ_land_zone_t zone);
civ_float_t civ_land_use_available_for(const civ_land_use_system_t *l, civ_land_zone_t zone);

#endif
