/* politics.c - Simple faction initialization and ownership assignment */
#include "../../include/core/environment/politics.h"
#include "../../include/core/environment/world.h"
#include "../../include/utils/utils.h"
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
    pol->factions[i].gold = 100.0f;
    pol->factions[i].military_strength = 50.0f;

    // Init diplomacy: Self is Ally, others Neutral
    for (int j = 0; j < MAX_FACTIONS; ++j) {
      if (i == j)
        pol->diplomacy[i][j] = RELATION_ALLY;
      else
        pol->diplomacy[i][j] = RELATION_NEUTRAL;
    }
  }
  // Start with no ownership
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      pol->ownership[y][x] = -1;
    }
  }
}

/* Helper: Calculate environmental stats for a faction */
typedef struct {
  float avg_temp;
  float avg_rain;
  float avg_elevation;
  int land_count;
} EnvStats;

static EnvStats calculate_env_stats(PoliticsState *pol, int faction_id,
                                    const GeographyState *geo,
                                    const ClimateState *clim) {
  EnvStats stats = {0};
  if (faction_id < 0)
    return stats;

  for (int y = 0; y < WORLD_HEIGHT; y++) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
      if (pol->ownership[y][x] == faction_id) {
        stats.avg_temp += clim->temperature[y][x];
        stats.avg_rain += clim->rainfall[y][x];
        stats.avg_elevation += geo->elevation[y][x];
        stats.land_count++;
      }
    }
  }

  if (stats.land_count > 0) {
    stats.avg_temp /= stats.land_count;
    stats.avg_rain /= stats.land_count;
    stats.avg_elevation /= stats.land_count;
  }
  return stats;
}

/* Very naive political assignment: assign a random faction to each land cell */
void politics_update(PoliticsState *pol, const GeographyState *geo,
                     const ClimateState *clim) {
  if (!pol || !geo || !clim)
    return;

  // existing random ownership logic (kept for init)
  // Check if initialization is needed (if all -1)
  if (pol->ownership[0][0] == -1) {
    // 1. Assign Territory
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
  } // End of Land Assignment

  // 2. Form/Update Governments (Runs every update, ensures every faction has a
  // gov)
  for (int i = 0; i < MAX_FACTIONS; i++) {
    // Only form if not exists
    if (pol->factions[i].government)
      continue;

    EnvStats stats = calculate_env_stats(pol, i, geo, clim);

    // If faction has no land, it can't form a government based on land
    if (stats.land_count == 0)
      continue;

    // Determinism:
    // High Temp (>25) OR Low Temp (< -5) -> Harsh -> Centralized
    // Moderate Temp -> Fertile -> Decentralized

    float harshness = 0.0f;
    if (stats.avg_temp > 25.0f)
      harshness += 0.5f; // Hot desert
    if (stats.avg_temp < -5.0f)
      harshness += 0.6f; // Frozen waste
    if (stats.avg_elevation > 1500.0f)
      harshness += 0.3f; // Mountains

    float fertility = 0.0f;
    if (stats.avg_rain > 5.0f && stats.avg_rain < 15.0f)
      fertility += 0.4f;
    if (stats.avg_temp > 10.0f && stats.avg_temp < 25.0f)
      fertility += 0.4f;

    // Base Name placeholder, will be overwritten by generator
    pol->factions[i].government = civ_custom_governance_create("gov", "Origin");

    // Apply Deterministic Stats
    civ_custom_governance_t *gov = pol->factions[i].government;

    if (harshness > 0.4f) {
      // Survival Mode
      gov->centralization = 0.8f + (harshness * 0.1f);
      gov->democracy_level = 0.2f - (harshness * 0.1f);
      pol->factions[i].military_strength = 0.8f; // Strong military needed
    } else if (fertility > 0.5f) {
      // Prosperity Mode
      gov->centralization = 0.3f;
      gov->democracy_level = 0.7f + (fertility * 0.1f);
      gov->efficiency = 0.8f;
    } else {
      // Balanced
      gov->centralization = 0.5f;
      gov->democracy_level = 0.5f;
    }

    // Clamp
    if (gov->centralization > 1.0f)
      gov->centralization = 1.0f;
    if (gov->democracy_level < 0.0f)
      gov->democracy_level = 0.0f;

    // Generate Name
    char name_buf[64];
    civ_custom_governance_generate_name(gov, name_buf, 64);

    // Update Faction Name to match Government Name
    snprintf(pol->factions[i].name, 64, "The %s", name_buf);
  }
}

void politics_tick(PoliticsState *pol, float time_delta) {
  if (!pol)
    return;

  for (int i = 0; i < MAX_FACTIONS; i++) {
    if (pol->factions[i].government) {
      // Evolve
      civ_custom_governance_update(pol->factions[i].government, time_delta);

      // Sync Faction Name if it changes (Dynamic!)
      // Currently update doesn't regen name, maybe we should check stats or
      // periodically regen? For simplicity, just regen.
      char name_buf[64];
      civ_custom_governance_generate_name(pol->factions[i].government, name_buf,
                                          64);
      snprintf(pol->factions[i].name, 64, "The %s", name_buf);
    }
  }
}

void politics_update_diplomacy(PoliticsState *pol) {
  if (!pol)
    return;

  // Simple simulation: Neighbors causing friction
  // Random chance to improve or worsen relations
  // This is just a stub for "simulation"
  for (int i = 0; i < MAX_FACTIONS; i++) {
    for (int j = i + 1; j < MAX_FACTIONS; j++) {
      RelationshipType current = pol->diplomacy[i][j];

      // Random flux
      int change = (rand() % 100);
      if (change < 5 && current != RELATION_WAR) {
        // Worsen
        pol->diplomacy[i][j] = (RelationshipType)(current - 1);
        pol->diplomacy[j][i] = (RelationshipType)(current - 1);
      } else if (change > 95 && current != RELATION_ALLY) {
        // Improve
        pol->diplomacy[i][j] = (RelationshipType)(current + 1);
        pol->diplomacy[j][i] = (RelationshipType)(current + 1);
      }
    }
  }
}

void politics_set_relationship(PoliticsState *pol, int faction_a, int faction_b,
                               RelationshipType type) {
  if (!pol)
    return;
  if (faction_a < 0 || faction_a >= MAX_FACTIONS)
    return;
  if (faction_b < 0 || faction_b >= MAX_FACTIONS)
    return;

  pol->diplomacy[faction_a][faction_b] = type;
  pol->diplomacy[faction_b][faction_a] = type;
}

RelationshipType politics_get_relationship(PoliticsState *pol, int faction_a,
                                           int faction_b) {
  if (!pol)
    return RELATION_NEUTRAL;
  if (faction_a < 0 || faction_a >= MAX_FACTIONS)
    return RELATION_NEUTRAL;
  if (faction_b < 0 || faction_b >= MAX_FACTIONS)
    return RELATION_NEUTRAL;

  return pol->diplomacy[faction_a][faction_b];
}
