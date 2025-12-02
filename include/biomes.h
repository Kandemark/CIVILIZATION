/* biomes.h - Biome classification system */
#ifndef BIOMES_H
#define BIOMES_H

#include "world.h"

/* Update terrain types based on elevation, temperature, and moisture */
void biomes_update(World *w);

#endif // BIOMES_H
