#ifndef CIV_WORLD_POLITICAL_BORDERS_H
#define CIV_WORLD_POLITICAL_BORDERS_H

#include "map_generator.h"
#include <stdbool.h>
#include <stdint.h>

bool civ_political_borders_load(const char *filepath, int32_t map_w, int32_t map_h);
void civ_political_borders_apply(civ_map_t *map);
void civ_political_borders_free(void);
const char *civ_political_borders_country_name(int16_t cid);
uint32_t    civ_political_borders_country_color(int16_t cid);
int16_t     civ_political_borders_tile_country(int32_t tx, int32_t ty, int32_t mw);

#endif
