/**
 * @file geography.c
 * @brief Implementation of geography system
 */

#define _USE_MATH_DEFINES
#include "../../../include/core/environment/geography.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


#define EARTH_RADIUS_KM 6371.0

civ_geography_t *civ_geography_create(const char *region_name,
                                      civ_coordinate_t sw,
                                      civ_coordinate_t ne) {
  civ_geography_t *geo = (civ_geography_t *)CIV_MALLOC(sizeof(civ_geography_t));
  if (!geo) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate geography");
    return NULL;
  }

  civ_geography_init(geo, region_name, sw, ne);
  return geo;
}

void civ_geography_destroy(civ_geography_t *geo) {
  if (!geo)
    return;
  CIV_FREE(geo->land_patches);
  CIV_FREE(geo);
}

void civ_geography_init(civ_geography_t *geo, const char *region_name,
                        civ_coordinate_t sw, civ_coordinate_t ne) {
  if (!geo || !region_name)
    return;

  memset(geo, 0, sizeof(civ_geography_t));
  strncpy(geo->region_name, region_name, sizeof(geo->region_name) - 1);
  geo->bounds_sw = sw;
  geo->bounds_ne = ne;
  geo->patch_capacity = 100;
  geo->land_patches = (civ_land_patch_t *)CIV_CALLOC(geo->patch_capacity,
                                                     sizeof(civ_land_patch_t));
}

static bool is_within_bounds(const civ_geography_t *geo,
                             civ_coordinate_t coord) {
  return (coord.latitude >= geo->bounds_sw.latitude &&
          coord.latitude <= geo->bounds_ne.latitude &&
          coord.longitude >= geo->bounds_sw.longitude &&
          coord.longitude <= geo->bounds_ne.longitude);
}

civ_result_t civ_geography_add_land_patch(civ_geography_t *geo,
                                          const civ_land_patch_t *patch) {
  civ_result_t result = {CIV_OK, NULL};

  if (!geo || !patch) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (!is_within_bounds(geo, patch->coordinate)) {
    result.error = CIV_ERROR_INVALID_ARGUMENT;
    result.message = "Patch outside region bounds";
    return result;
  }

  if (geo->patch_count >= geo->patch_capacity) {
    geo->patch_capacity *= 2;
    geo->land_patches = (civ_land_patch_t *)CIV_REALLOC(
        geo->land_patches, geo->patch_capacity * sizeof(civ_land_patch_t));
  }

  if (geo->land_patches) {
    geo->land_patches[geo->patch_count++] = *patch;
  }

  return result;
}

civ_float_t civ_geography_calculate_distance(civ_coordinate_t a,
                                             civ_coordinate_t b) {
  /* Haversine formula for great-circle distance */
  civ_float_t lat1 = a.latitude * M_PI / 180.0f;
  civ_float_t lon1 = a.longitude * M_PI / 180.0f;
  civ_float_t lat2 = b.latitude * M_PI / 180.0f;
  civ_float_t lon2 = b.longitude * M_PI / 180.0f;

  civ_float_t dlat = lat2 - lat1;
  civ_float_t dlon = lon2 - lon1;

  civ_float_t sin_dlat = sin(dlat / 2.0f);
  civ_float_t sin_dlon = sin(dlon / 2.0f);

  civ_float_t a_val =
      sin_dlat * sin_dlat + cos(lat1) * cos(lat2) * sin_dlon * sin_dlon;
  civ_float_t c = 2.0f * atan2(sqrt(a_val), sqrt(1.0f - a_val));

  return EARTH_RADIUS_KM * c;
}

civ_float_t civ_geography_get_agricultural_area(const civ_geography_t *geo) {
  if (!geo)
    return 0.0f;

  civ_float_t total = 0.0f;
  for (size_t i = 0; i < geo->patch_count; i++) {
    if (geo->land_patches[i].land_use == CIV_LAND_USE_AGRICULTURE) {
      total += geo->land_patches[i].area;
    }
  }

  return total;
}
