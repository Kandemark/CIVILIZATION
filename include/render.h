/* render.h - Functions to output PPM images for each map view */
#ifndef RENDER_H
#define RENDER_H

#include "world.h"

/* Render a grayscale elevation map */
void render_geography(const World *w, const char *filename);

/* Render political map with faction colors */
void render_politics(const World *w, const char *filename);

/* Render climate map (temperature gradient) */
void render_climate(const World *w, const char *filename);

/* Render biome map (desert vs other biomes) */
void render_biomes(const World *w, const char *filename);

#endif // RENDER_H
