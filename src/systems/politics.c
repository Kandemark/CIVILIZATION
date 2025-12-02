/* politics.c - Simple faction initialization and ownership assignment */
#include "politics.h"
#include "geography.h"
#include "utils.h"
#include <string.h>

/* Helper to generate a deterministic color for a faction based on its id */
static void generate_faction_color(int id, unsigned char out[3]) {
  // Simple pseudo‑random but deterministic mapping using a small LCG
  unsigned int seed = (unsigned int)(id * 2654435761u);
  out[0] = (seed >> 16) & 0xFF; // R
  out[1] = (seed >> 8) & 0xFF;  // G
  out[2] = seed & 0xFF;         // B
  // Avoid very dark colors (make sure at least one channel > 64)
  if (out[0] < 64 && out[1] < 64 && out[2] < 64) {
    out[0] += 64;
  }
}

void politics_initialize(PoliticsState *pol, UtilsState *utils) {
  if (!pol)
    return;
  // Initialise factions with deterministic names and colors
  for (int i = 0; i < MAX_FACTIONS; ++i) {
    pol->factions[i].id = i;
    snprintf(pol->factions[i].name, sizeof(pol->factions[i].name), "Faction_%d",
             i);
    generate_faction_color(i, pol->factions[i].color);
  }
  // Start with no ownership
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      pol->ownership[y][x] = -1;
    }
  }
}

/* Very naive political assignment: assign a random faction to each land cell */
void politics_update(PoliticsState *pol, const GeographyState *geo) {
  if (!pol || !geo)
    return;
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      if (geo->water[y][x]) {
        pol->ownership[y][x] = -1; // oceans have no owner
        continue;
      }
      // Deterministic pseudo‑random based on coordinates
      unsigned int seed = (unsigned int)(x * 73856093u ^ y * 19349663u);
      int faction = seed % MAX_FACTIONS;
      pol->ownership[y][x] = faction;
    }
  }
}
