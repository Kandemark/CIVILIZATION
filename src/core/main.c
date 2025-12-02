/* main.c - Demo of the World Simulation Engine */
#include "render.h"
#include "world.h"
#include <stdio.h>


int main(void) {
  printf("Initializing World...\n");
  World *w = world_create();
  if (!w) {
    fprintf(stderr, "Failed to create world.\n");
    return 1;
  }

  world_initialize(w);

  printf("Running Simulation Step...\n");
  world_update(w);

  printf("Rendering Maps...\n");
  render_geography(w, "geography.ppm");
  render_politics(w, "politics.ppm");
  render_climate(w, "climate.ppm");
  render_biomes(w, "biomes.ppm");

  printf("Cleaning up...\n");
  world_destroy(w);

  printf("Done. Check .ppm files.\n");
  return 0;
}
