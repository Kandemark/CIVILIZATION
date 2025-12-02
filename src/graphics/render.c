/* render.c - Implementation of map rendering to PPM */
#include "render.h"
#include <stdio.h>

/* Helper to write PPM header */
static void write_ppm_header(FILE *f, int width, int height) {
  fprintf(f, "P3\n%d %d\n255\n", width, height);
}

/* Helper to write a pixel */
static void write_pixel(FILE *f, int r, int g, int b) {
  fprintf(f, "%d %d %d ", r, g, b);
}

void render_geography(const World *w, const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return;
  write_ppm_header(f, WORLD_WIDTH, WORLD_HEIGHT);

  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      if (w->geo.water[y][x]) {
        // Ocean blue
        write_pixel(f, 0, 0, 128);
      } else if (w->geo.river_volume[y][x] > 5.0f) {
        // River blue (lighter than ocean)
        write_pixel(f, 0, 100, 255);
      } else {
        // Grayscale elevation
        float elev = w->geo.elevation[y][x];
        int val = (int)((elev / MAX_ELEVATION) * 255.0f);
        if (val < 0)
          val = 0;
        if (val > 255)
          val = 255;
        write_pixel(f, val, val, val);
      }
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void render_politics(const World *w, const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return;
  write_ppm_header(f, WORLD_WIDTH, WORLD_HEIGHT);

  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      int owner = w->pol.ownership[y][x];
      if (w->geo.water[y][x]) {
        write_pixel(f, 0, 0, 128); // Sea
      } else if (owner >= 0 && owner < MAX_FACTIONS) {
        const unsigned char *c = w->pol.factions[owner].color;
        write_pixel(f, c[0], c[1], c[2]);
      } else {
        write_pixel(f, 200, 200, 200); // Unclaimed land
      }
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void render_climate(const World *w, const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return;
  write_ppm_header(f, WORLD_WIDTH, WORLD_HEIGHT);

  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      float temp = w->clim.temperature[y][x];
      // Map -30..40 to 0..255 (Blue..Red)
      float t = (temp - MIN_TEMPERATURE) / (MAX_TEMPERATURE - MIN_TEMPERATURE);
      if (t < 0.0f)
        t = 0.0f;
      if (t > 1.0f)
        t = 1.0f;

      int r = (int)(t * 255.0f);
      int b = (int)((1.0f - t) * 255.0f);
      write_pixel(f, r, 0, b);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void render_biomes(const World *w, const char *filename) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return;
  write_ppm_header(f, WORLD_WIDTH, WORLD_HEIGHT);

  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      WorldCell *cell = &w->cells[y][x];
      if (cell->terrain == TERRAIN_OCEAN) {
        write_pixel(f, 0, 0, 128);
      } else if (cell->river_volume > 5.0f) {
        write_pixel(f, 0, 100, 255); // River
      } else {
        switch (cell->terrain) {
        case TERRAIN_DESERT:
          write_pixel(f, 240, 230, 140); // Sandy
          break;
        case TERRAIN_FOREST:
          write_pixel(f, 34, 139, 34); // Forest green
          break;
        case TERRAIN_TUNDRA:
          write_pixel(f, 200, 200, 200); // Greyish
          break;
        case TERRAIN_SNOW:
          write_pixel(f, 255, 255, 255); // White
          break;
        case TERRAIN_MOUNTAINS:
          write_pixel(f, 100, 100, 100); // Dark grey
          break;
        case TERRAIN_HILLS:
          write_pixel(f, 160, 160, 100); // Brownish green
          break;
        default:                        // PLAINS
          write_pixel(f, 107, 142, 35); // Olive drab
          break;
        }
      }
    }
    fprintf(f, "\n");
  }
  fclose(f);
}
